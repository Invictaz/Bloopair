/*
 *   Copyright (C) 2021 GaryOderNichts
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <imports.h>

typedef struct {
  uint8_t ft[8];
  uint8_t sb[8];
} CryptoState;

void cryptoInit(CryptoState* vals, const uint8_t* key);
void encrypt(const CryptoState* vals, uint8_t* encrypted, const uint8_t* decrypted, uint32_t addr_offset, uint32_t length);
void decrypt(const CryptoState* vals, uint8_t* decrypted, const uint8_t* encrypted, uint32_t addr_offset, uint32_t length);
