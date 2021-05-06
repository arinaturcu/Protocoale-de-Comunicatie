#include "skel.h"

int interfaces[ROUTER_NUM_INTERFACES];
struct route_table_entry *rtable;
int rtable_size;

struct arp_entry *arp_table;
int arp_table_len;

/*
 Returns a pointer (eg. &rtable[i]) to the best matching route
 for the given dest_ip. Or NULL if there is no matching route.
*/
struct route_table_entry *get_best_route(__u32 dest_ip) {
	/* TODO 1: Implement the function */

	for (int i = 0; i < rtable_size; ++i) {
		if ((dest_ip & rtable[i].mask) == (rtable[i].mask & rtable[i].prefix)) {
			return &rtable[i];
		}
	}

	return NULL;
}

/*
 Returns a pointer (eg. &arp_table[i]) to the best matching ARP entry.
 for the given dest_ip or NULL if there is no matching entry.
*/
struct arp_entry *get_arp_entry(__u32 ip) {
    /* TODO 2: Implement */

	for (int i = 0; i < arp_table_len; ++i) {
		if (ip == arp_table[i].ip) {
			return &arp_table[i];
		}
	}

    return NULL;
}

static void sort_rtable() {
	for (int i = 0; i < rtable_size - 1; ++i) {
		for (int j = i + 1; j < rtable_size; ++j) {
			if (ntohl(rtable[j].mask) > ntohl(rtable[i].mask)) {
				struct route_table_entry tmp;
				memcpy(&tmp, &rtable[j], sizeof(struct route_table_entry));
				memcpy(&rtable[j], &rtable[i], sizeof(struct route_table_entry));
				memcpy(&rtable[i], &tmp, sizeof(struct route_table_entry));
			}
		}
	}
}

int main(int argc, char *argv[])
{
	msg m;
	int rc;

	init();
	rtable = malloc(sizeof(struct route_table_entry) * 100);
	arp_table = malloc(sizeof(struct  arp_entry) * 100);
	DIE(rtable == NULL, "memory");
	rtable_size = read_rtable(rtable);
	parse_arp_table();

	/* Students will write code here */
	sort_rtable();

	while (1) {
		rc = get_packet(&m);
		DIE(rc < 0, "get_message");

		struct ether_header *eth_hdr = (struct ether_header *)m.payload;
		struct iphdr *ip_hdr = (struct iphdr *)(m.payload + sizeof(struct ether_header));

		/* Check if packet type is ETHERNET */
		if(ntohs(eth_hdr->ether_type )!= ETHERTYPE_IP) {
			continue;
		}

		/* TODO 3: Check the checksum */
		if (ip_checksum(ip_hdr, sizeof(struct iphdr)) != 0) continue;

		/* TODO 4: Check TTL >= 1 */
		if (ip_hdr->ttl <= 0) continue;

		/* TODO 5: Find best matching route (using the function you wrote at TODO 1) */
		struct route_table_entry *entry = get_best_route(ip_hdr->daddr);

		/* TODO 6: Update TTL and recalculate the checksum */
		ip_hdr->ttl--;
		ip_hdr->check = 0;
		ip_hdr->check = ip_checksum(ip_hdr, sizeof(struct iphdr));

		/* TODO 7: Find matching entry and update Ethernet addresses */
		struct arp_entry *a_entry = get_arp_entry(ip_hdr->daddr);
		if (a_entry == NULL) continue;

		memcpy(eth_hdr->ether_dhost, a_entry->mac, 6);
		get_interface_mac(entry->interface, eth_hdr->ether_shost);

		/* TODO 8: Forward the pachet to best_route->interface */
		send_packet(entry->interface, &m);
	}
}
