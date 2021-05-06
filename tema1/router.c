#include <queue.h>
#include "skel.h"
#include "parser.h"

#define ARP_MAXSIZE 100
#define MAX_LINE_LEN 100

struct route_table_entry *rtable;
int rtable_size;

struct arp_table_entry *arp_table;
int arp_table_size;

queue packets;

/*
 * Binary search
 * Returns a pointer (eg. &rtable[i]) to the best matching route
 * for the given dest_ip. Or NULL if there is no matching route.
 */
struct route_table_entry *get_best_route(__u32 dest_ip, int start, int end)
{
	if (start > end)
		return NULL;
	int mid = (start + end) / 2;

	if ((dest_ip & rtable[mid].mask) == (rtable[mid].mask & rtable[mid].prefix))
	{
		return &rtable[mid];
	}
	else if ((dest_ip & rtable[mid].mask) > (rtable[mid].mask & rtable[mid].prefix))
	{
		return get_best_route(dest_ip, start, mid - 1);
	}
	else
	{
		return get_best_route(dest_ip, mid + 1, end);
	}

	printf("Can't find best route\n");
	return NULL;
}

struct arp_table_entry *get_arp_entry(__u32 ip)
{
	for (int i = 0; i < arp_table_size; ++i)
	{
		if (ip == arp_table[i].ip)
		{
			return &arp_table[i];
		}
	}

	return NULL;
}

int compar(const void *r1, const void *r2)
{
	return (((struct route_table_entry *)r2)->mask & ((struct route_table_entry *)r2)->prefix) 
		- (((struct route_table_entry *)r1)->mask & ((struct route_table_entry *)r1)->prefix);
}

static void sort_rtable()
{
	qsort(rtable, rtable_size, sizeof(struct route_table_entry), compar);
}

static int count_rtable_entries(FILE *rtable_file)
{
	char line[MAX_LINE_LEN];
	int count = 0;

	while (fgets(line, sizeof(line), rtable_file)) count++;

	return count;
}

static void forward_packet(packet *m)
{
	struct ether_header *eth_hdr = (struct ether_header *)m->payload;
	struct iphdr *ip_hdr = (struct iphdr *)(m->payload + sizeof(struct ether_header));

	printf("IP PACKET\n");
	/* Check the checksum */
	if (ip_checksum(ip_hdr, sizeof(struct iphdr)) != 0)
	{
		fprintf(stderr, "Wrong checksum\n");
		return;
	}

	/* Time exceeded */
	if (ip_hdr->ttl <= 1)
	{
		fprintf(stderr, "Time Exceeded\n");
		send_icmp_error(ip_hdr->saddr, ip_hdr->daddr, eth_hdr->ether_dhost, eth_hdr->ether_shost, 
			ICMP_TIME_EXCEEDED, 0, m->interface);
		return;
	}

	/* Find best matching route */
	struct route_table_entry *best_route = get_best_route(ip_hdr->daddr, 0, rtable_size - 1);

	/* Destination unreacheable */
	if (best_route == NULL)
	{
		fprintf(stderr, "Destination Unreacheable\n");
		send_icmp_error(ip_hdr->saddr, ip_hdr->daddr, eth_hdr->ether_dhost, eth_hdr->ether_shost, 
			ICMP_DEST_UNREACH, 0, m->interface);
		return;
	}

	/* Update TTL and recalculate the checksum */
	ip_hdr->ttl--;
	ip_hdr->check = 0;
	ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));

	/* Find matching entry and update Ethernet addresses */
	struct arp_table_entry *a_entry = get_arp_entry(best_route->next_hop);

	/* Enqueue the packet and send an ARP request */
	if (a_entry == NULL)
	{
		/* Enqueue the packet for after receiving the ARP reply */
		packet m_copy;
		memcpy(&m_copy, m, sizeof(packet));
		queue_enq(packets, &m_copy);

		struct in_addr best_route_addr;
		inet_aton(get_interface_ip(best_route->interface), &best_route_addr);

		struct ether_header new_eth_hdr;
		uint8_t best_route_mac[6];
		uint8_t broadcast_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

		get_interface_mac(best_route->interface, best_route_mac);
		build_ethhdr(&new_eth_hdr, best_route_mac, broadcast_mac, htons(ETHERTYPE_ARP));

		send_arp(best_route->next_hop, best_route_addr.s_addr, &new_eth_hdr, best_route->interface, htons(ARPOP_REQUEST));
		return;
	}

	memcpy(eth_hdr->ether_dhost, a_entry->mac, sizeof(eth_hdr->ether_dhost));

	get_interface_mac(best_route->interface, eth_hdr->ether_shost);

	/* Forward the pachet to best_route->interface */
	send_packet(best_route->interface, m);
}

int main(int argc, char *argv[])
{
	packet m;
	int rc;
	int arp_max_size = ARP_MAXSIZE;

	init(argc - 2, argv + 2);

	FILE *rtable_file = fopen(argv[1], "r");

	rtable = malloc(sizeof(struct route_table_entry) * count_rtable_entries(rtable_file));
	DIE(rtable == NULL, "Couldn't alloc memory for routing table");

	arp_table = malloc(sizeof(struct arp_table_entry) * arp_max_size);
	DIE(arp_table == NULL, "Couldn't alloc memory for ARP table");

	fseek(rtable_file, 0, SEEK_SET);
	rtable_size = read_rtable(rtable, fopen(argv[1], "r"));

	sort_rtable();

	packets = queue_create();

	while (1)
	{
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

		struct icmphdr *icmp_hdr = parse_icmp(eth_hdr);
		struct arp_header *arp_hdr = parse_arp(eth_hdr);

		/* Handle IP packets */
		if (ntohs(eth_hdr->ether_type) == ETHERTYPE_IP)
		{
			/* Handle ICMP */
			if (icmp_hdr)
			{
				/* Check if packet is for me */
				struct in_addr tmp_addr;
				inet_aton(get_interface_ip(m.interface), &tmp_addr);

				if (ip_hdr->daddr == tmp_addr.s_addr)
				{
					if (icmp_hdr->type == ICMP_ECHO)
					{
						send_icmp(ip_hdr->saddr, ip_hdr->daddr, eth_hdr->ether_dhost, eth_hdr->ether_shost, ICMP_ECHOREPLY,
								  0, m.interface, icmp_hdr->un.echo.id, icmp_hdr->un.echo.sequence);
					}

					continue;
				}
			}

			/* Forward an ordinary IP packet */
			forward_packet(&m);
			continue;
		}

		/* Handle ARP packets */
		if (arp_hdr)
		{
			/* Send ARP reply */
			if (ntohs(arp_hdr->op) == ARPOP_REQUEST)
			{
				struct in_addr add;
				inet_aton(get_interface_ip(m.interface), &add);

				if (arp_hdr->tpa == add.s_addr)
				{
					uint8_t mac[6];
					get_interface_mac(m.interface, mac);

					memcpy(eth_hdr->ether_dhost, eth_hdr->ether_shost, sizeof(eth_hdr->ether_dhost));
					memcpy(eth_hdr->ether_shost, mac, sizeof(eth_hdr->ether_shost));

					send_arp(arp_hdr->spa, arp_hdr->tpa, eth_hdr, m.interface, htons(ARPOP_REPLY));
				}
			}
			/* Handle ARP reply */
			else
			{	
				/* Update the ARP table with the new data */
				arp_table[arp_table_size].ip = arp_hdr->spa;
				memcpy(arp_table[arp_table_size].mac, arp_hdr->sha, sizeof(arp_table[arp_table_size].mac));

				arp_table_size++;

				/* Realloc if necessary */
				if (arp_table_size == arp_max_size)
				{
					arp_max_size = arp_max_size * 2;
					arp_table = realloc(arp_table, arp_max_size);
				}
				
				/* Now that the destination is known, send the packet */
				if (!queue_empty(packets)) 
				{
					packet *to_send = queue_deq(packets);
					forward_packet(to_send);
				}
			}
		}
	}
}
