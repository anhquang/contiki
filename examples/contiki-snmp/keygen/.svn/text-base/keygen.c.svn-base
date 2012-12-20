/* -----------------------------------------------------------------------------
 * SNMP implementation for Contiki
 *
 * Copyright (C) 2010 Siarhei Kuryla <kurilo@gmail.com>
 *
 * This program is part of free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#include <string.h>
#include "md5.h"
#include <stdio.h>

/*
 * Taken from rfc 3414
 */
void password_to_key_md5(u8t *pwd, u8t pwd_len, u8t *engineID, u8t engineLength, u8t *key)
{
    MD5_CTX     MD;
    u8t     *cp, password_buf[64];
    u32t      password_index = 0;
    u32t      count = 0, i;
    memset(&MD, 0, sizeof(MD));
    MD5Init (&MD);
    memset(key, 0, 16);
    
    while (count < 1048576) {
        cp = password_buf;
        for (i = 0; i < 64; i++) {
            *cp++ = pwd[password_index++ % pwd_len];
        }
        MD5Update (&MD, password_buf, 64);
        count += 64;
    }
    MD5Final (&MD, key);

    memcpy(password_buf, key, 16);
    memcpy(password_buf + 16, engineID, engineLength);
    memcpy(password_buf + 16 + engineLength, key, 16);

    memset(&MD, 0, sizeof(MD));
    MD5Init(&MD);
    MD5Update(&MD, password_buf, 32 + engineLength);
    MD5Final(&MD, key);
}

u8t kul[16];

u8t engineID_array[] = {0x80, 0x00, 0x1f, 0x88, 0x80, 0x77, 0xd5, 0xcb, 0x77, 0x9e, 0xa0, 0xef, 0x4b};
ptr_t engineID = {engineID_array, 13};

void main(int argc, char* argv[]) {
    printf("%d\n", argc);
    if (argc == 2) {
    } else {
         printf("error: wrong number of arguments\n\n");
         printf("the input format:\n");
         printf("./keygen <password>\n\n");
         return;
    }
    password_to_key_md5(argv[1], strlen((char*)argv[1]), engineID.ptr, engineID.len, kul);
}
