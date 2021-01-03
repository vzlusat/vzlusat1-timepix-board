/*
Cubesat Space Protocol - A small network-layer protocol designed for Cubesats
Copyright (C) 2012 GomSpace ApS (http://www.gomspace.com)
Copyright (C) 2012 AAUSAT3 Project (http://aausat3.space.aau.dk)

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdint.h>
#include <string.h>
#include <inttypes.h>

#include <csp/csp.h>
#include <csp/csp_endian.h>

#include "equalization.h"

#ifdef CSP_USE_CRC32

uint32_t csp_crc32_memory(const uint8_t * data, uint32_t length) {
   uint32_t crc;

   crc = 0xFFFFFFFF;
   while (length--)
#ifdef __AVR__
	   crc = pgm_read_dword(&crc_tab[(crc ^ *data++) & 0xFFL]) ^ (crc >> 8);
#else
	   crc = crc_tab[(crc ^ *data++) & 0xFFL] ^ (crc >> 8);
#endif

   return (crc ^ 0xFFFFFFFF);
}

int csp_crc32_append(csp_packet_t * packet) {

	uint32_t crc;

	/* NULL pointer check */
	if (packet == NULL)
		return CSP_ERR_INVAL;

	/* Calculate CRC32, convert to network byte order */
	crc = csp_crc32_memory(packet->data, packet->length);
	crc = csp_hton32(crc);

	/* Copy checksum to packet */
	memcpy(&packet->data[packet->length], &crc, sizeof(uint32_t));
	packet->length += sizeof(uint32_t);

	return CSP_ERR_NONE;

}

int csp_crc32_verify(csp_packet_t * packet) {

	uint32_t crc;

	/* NULL pointer check */
	if (packet == NULL)
		return CSP_ERR_INVAL;

	if (packet->length < sizeof(uint32_t))
		return CSP_ERR_INVAL;

	/* Calculate CRC32, convert to network byte order */
	crc = csp_crc32_memory(packet->data, packet->length - sizeof(uint32_t));
	crc = csp_hton32(crc);

	/* Compare calculated checksum with packet header */
	if (memcmp(&packet->data[packet->length] - sizeof(uint32_t), &crc, sizeof(uint32_t)) != 0) {
		/* CRC32 failed */
		return CSP_ERR_INVAL;
	} else {
		/* Strip CRC32 */
		packet->length -= sizeof(uint32_t);
		return CSP_ERR_NONE;
	}

}

#endif // CSP_USE_CRC32
