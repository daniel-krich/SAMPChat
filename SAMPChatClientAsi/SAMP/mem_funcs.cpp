/*
	PROJECT:		mod_sa
	LICENSE:		See LICENSE in the top level directory
	COPYRIGHT:		Copyright we_sux, FYP
	mod_sa is available from http://code.google.com/p/m0d-s0beit-sa/
	mod_sa is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	mod_sa is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with mod_sa.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "../main.h"

bool isBadPtr_handlerAny(void* pointer, ULONG size, DWORD dwFlags)
{
	DWORD						dwSize;
	MEMORY_BASIC_INFORMATION	meminfo;

	if (NULL == pointer)
		return true;

	memset(&meminfo, 0x00, sizeof(meminfo));
	dwSize = VirtualQuery(pointer, &meminfo, sizeof(meminfo));

	if (0 == dwSize)
		return true;

	if (MEM_COMMIT != meminfo.State)
		return true;

	if (0 == (meminfo.Protect & dwFlags))
		return true;

	if (size > meminfo.RegionSize)
		return true;

	if ((unsigned)((char*)pointer - (char*)meminfo.BaseAddress) > (unsigned)(meminfo.RegionSize - size))
		return true;

	return false;
}

bool isBadPtr_readAny(void* pointer, ULONG size)
{
	return isBadPtr_handlerAny(pointer, size, PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ |
		PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
}

bool isBadPtr_writeAny(void* pointer, ULONG size)
{
	return isBadPtr_handlerAny(pointer, size,
		PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY);
}

uint32_t GetFromPool(DWORD value, DWORD Pool, DWORD function)
{
	uint32_t	retval;
	__asm
	{
		mov ecx, Pool
		mov ecx, [ecx]
		push value
		call function
		mov retval, eax
	}

	return retval;
}

void gta_weather_state_set(int state)
{
	*(uint16_t*)0x00C81318 = (uint16_t)state;
	*(uint16_t*)0x00C8131C = (uint16_t)state;
	*(uint16_t*)0x00C81320 = (uint16_t)state;
}

void gta_time_hour_set(int hour)
{
	*(uint8_t*)0x00B70153 = (uint8_t)hour;
}

void gta_money_set(uint32_t value)
{
	*(uint32_t*)0x00B7CE50 = value;
	*(uint32_t*)0x00B7CE54 = value;
}

uint32_t gta_money_get(void)
{
	return *(uint32_t*)0x00B7CE50;
}

void gta_game_speed_set(float value)
{
	*(float*)0x00B7CB64 = value;
}

float gta_game_speed_get(void)
{
	return *(float*)0x00B7CB64;
}

struct checkpoint* gta_checkpoint_info_get(int n)
{
	struct checkpoint* cp = (struct checkpoint*)0x00C7F158;

	if (vect3_near_zero(cp->position))
		return NULL;

	return &cp[n];
}

int gta_menu_active(void)
{
	return (int)(*(uint8_t*)0x00BA67A4);
}

void gta_menu_active_set(int enabled)
{
	/* untested */
	*(uint8_t*)0x00BA67A4 = (uint8_t)enabled;
}

int gta_weapon_ammo_set(struct actor_info* info, int slot, int ammo)
{
	uint32_t	ammo_old = info->weapon[slot].ammo;

	if (ammo >= 0)
		info->weapon[slot].ammo = ammo;

	return ammo_old;
}

int gta_weapon_ammo_clip_set(struct actor_info* info, int slot, int ammo_clip)
{
	uint32_t	ammo_clip_old = info->weapon[slot].ammo_clip;

	if (ammo_clip >= 0)
		info->weapon[slot].ammo_clip = ammo_clip;

	return ammo_clip_old;
}

int gta_interior_id_get(void)
{
	return (int)*(uint32_t*)0x00B72914;
}

// ---------------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------
static int __page_size_get(void)
{
	static int	page_size = -1;
	SYSTEM_INFO si;

	if (page_size == -1)
	{
		GetSystemInfo(&si);
		page_size = (int)si.dwPageSize;
	}

	return page_size;
}

static int __page_write(void* _dest, const void* _src, uint32_t len)
{
	static int		page_size = __page_size_get();
	uint8_t* dest = (uint8_t*)_dest;
	const uint8_t* src = (const uint8_t*)_src;
	DWORD			prot_prev = 0;
	int				prot_changed = 0;
	SIZE_T			write_len;
	int				ret = 1;

	while (len > 0)
	{
		int page_offset = (int)((UINT_PTR)dest % page_size);
		int page_remain = page_size - page_offset;
		int this_len = len;

		if (this_len > page_remain)
			this_len = page_remain;

		if (isBadPtr_writeAny(dest, this_len))
		{
			if (!VirtualProtect((void*)dest, this_len, PAGE_EXECUTE_READWRITE, &prot_prev))
				ret = 0;
			else
				prot_changed = 1;
		}

		if (!WriteProcessMemory(GetCurrentProcess(), dest, (void*)src, this_len, &write_len))
			write_len = 0;

		if (prot_changed)
		{
			DWORD	dummy;
			VirtualProtect((void*)dest, this_len, prot_prev, &dummy);
		}

		if ((int)write_len != this_len)
			ret = 0;

		dest += this_len;
		src += this_len;
		len -= this_len;
	}

	return ret;
}

static int __page_read(void* _dest, const void* _src, uint32_t len)
{
	static int	page_size = __page_size_get();
	uint8_t* dest = (uint8_t*)_dest;
	uint8_t* src = (uint8_t*)_src;
	DWORD		prot_prev = 0;
	int			prot_changed = 0;
	SIZE_T		read_len;
	int			ret = 1;

	while (len > 0)
	{
		int page_offset = (int)((UINT_PTR)src % page_size);
		int page_remain = page_size - page_offset;
		int this_len = len;

		if (this_len > page_remain)
			this_len = page_remain;

		if (isBadPtr_readAny(src, this_len))
		{
			if (!VirtualProtect((void*)src, this_len, PAGE_EXECUTE_READWRITE, &prot_prev))
				ret = 0;
			else
				prot_changed = 1;
		}

		if (!ReadProcessMemory(GetCurrentProcess(), src, dest, this_len, &read_len))
			read_len = 0;

		if (prot_changed)
		{
			DWORD	dummy;
			VirtualProtect((void*)src, this_len, prot_prev, &dummy);
		}

		if ((int)read_len != this_len)
		{
			memset(dest + read_len, 0, this_len - read_len);
			ret = 0;
		}

		dest += this_len;
		src += this_len;
		len -= this_len;
	}

	return ret;
}

int memcpy_safe(void* _dest, const void* _src, uint32_t len, int check, const void* checkdata)
{
	static int		page_size = __page_size_get();
	static int		recurse_ok = 1;
	uint8_t			buf[4096];
	uint8_t* dest = (uint8_t*)_dest;
	const uint8_t* src = (const uint8_t*)_src;
	int				ret = 1;

	if (check && checkdata)
	{
		if (!memcmp_safe(checkdata, _dest, len))
			return 0;
	}

	while (len > 0)
	{
		uint32_t	this_len = sizeof(buf);

		if (this_len > len)
			this_len = len;

		if (!__page_read(buf, src, this_len))
			ret = 0;

		if (!__page_write(dest, buf, this_len))
			ret = 0;

		len -= this_len;
		src += this_len;
		dest += this_len;
	}

	return ret;
}

int memset_safe(void* _dest, int c, uint32_t len)
{
	uint8_t* dest = (uint8_t*)_dest;
	uint8_t buf[4096];

	memset(buf, c, (len > 4096) ? 4096 : len);

	for (;; )
	{
		if (len > 4096)
		{
			if (!memcpy_safe(dest, buf, 4096))
				return 0;
			dest += 4096;
			len -= 4096;
		}
		else
		{
			if (!memcpy_safe(dest, buf, len))
				return 0;
			break;
		}
	}

	return 1;
}

int memcmp_safe(const void* _s1, const void* _s2, uint32_t len)
{
	const uint8_t* s1 = (const uint8_t*)_s1;
	const uint8_t* s2 = (const uint8_t*)_s2;
	uint8_t			buf[4096];

	for (;; )
	{
		if (len > 4096)
		{
			if (!memcpy_safe(buf, s1, 4096))
				return 0;
			if (memcmp(buf, s2, 4096))
				return 0;
			s1 += 4096;
			s2 += 4096;
			len -= 4096;
		}
		else
		{
			if (!memcpy_safe(buf, s1, len))
				return 0;
			if (memcmp(buf, s2, len))
				return 0;
			break;
		}
	}

	return 1;
}

void* dll_baseptr_get(const char* dll_name)
{
	return GetModuleHandleA(dll_name);
}

void str_split_free(struct str_split* split)
{
	if (split != NULL)
	{
		if (split->str != NULL)
			free(split->str);
		if (split->argv != NULL)
			free(split->argv);
		free(split);
	}
}

struct str_split* str_split(const char* str, const char* ch)
{
	struct str_split* split;
	char* prev, * next;
	char* dest;
	void* tmp;

	/* left trim */
	while (*str && strchr(ch, *str) != NULL)
		str++;

	split = (struct str_split*)calloc(1, sizeof(struct str_split));
	if (split == NULL)
		return NULL;

	split->str = _strdup(str);
	if (split->str == NULL)
	{
		free(split);
		return NULL;
	}

	for (prev = split->str;; prev = next)
	{
		if (*prev == '"')
		{
			/* find the ending " */
			for (dest = next = ++prev; *next; next++)
			{
				if (*next == '"')
				{
					next++;
					break;
				}
				else if (*next == '\\' && next[1] != 0)
					*dest++ = *++next;
				else
					*dest++ = *next;
			}

			*dest = 0;
		}
		else
		{
			next = prev;
		}

		/* find next value */
		for (; *next && strchr(ch, *next) == NULL; next++);
		next = *next ? next : NULL;

		if ((tmp = realloc(split->argv, (split->argc + 1) * sizeof(char*))) == NULL)
			goto out;
		split->argv = (char**)tmp;

		split->argv[split->argc] = prev;
		split->argc++;

		if (next == NULL)
			break;

		for (*next++ = 0; *next && strchr(ch, *next) != NULL; next++);

		if (*next == 0)
			break;
	}

	tmp = realloc(split->argv, (split->argc + 1) * sizeof(char*));
	if (tmp == NULL)
		goto out;
	split->argv = (char**)tmp;

	split->argv[split->argc] = NULL;

	return split;

out:;
	str_split_free(split);
	return NULL;
}

size_t strlcpy(char* dst, const char* src, size_t size)
{
	size_t	len = strlen(src);

	if (size == 0)
		return len;

	if (len >= size)
	{
		size--;
		memcpy(dst, src, size);
		dst[size] = 0;
	}
	else if (size > 0)
	{
		strcpy(dst, src);
	}

	return len;
}

size_t strlcat(char* dst, const char* src, size_t size)
{
	size_t	dlen = strlen(dst);
	size_t	slen = strlen(src);

	if (size == 0)
		return dlen + slen;

	if (dlen + slen >= size)
	{
		size -= dlen - 1;
		memcpy(dst + dlen, src, size);
		dst[dlen + size] = 0;
	}
	else if (size > 0)
	{
		strcpy(dst + dlen, src);
	}

	return dlen + slen;
}

void* memdup(const void* src, int len)
{
	void* dest = malloc(len);

	if (dest != NULL)
		memcpy(dest, src, len);

	return dest;
}

static signed char hex_to_dec(signed char ch)
{
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f')
		return ch - 'A' + 10;

	return -1;
}

uint8_t* hex_to_bin(const char* str)
{
	int		len = (int)strlen(str);
	uint8_t* buf, * sbuf;

	if (len == 0 || len % 2 != 0)
		return NULL;

	sbuf = buf = (uint8_t*)malloc(len / 2);

	while (*str)
	{
		signed char bh = hex_to_dec(*str++);
		signed char bl = hex_to_dec(*str++);

		if (bl == -1 || bh == -1)
		{
			free(sbuf);
			return NULL;
		}

		*buf++ = (uint8_t)(bl | (bh << 4));
	}

	return sbuf;
}

D3DCOLOR hex_to_color(const char* str, int len)
{
	char buf[12];
	strncpy_s(buf, str, len);
	D3DCOLOR color = 0x00;
	byte* colorByteSet = (byte*)&color;
	int stri = 0;
	for (int i = sizeof(color) - 1; i >= 0; i--)
	{
		if (i == 3 && len == 6)
		{
			colorByteSet[3] = 0xFF;
		}
		else
		{
			signed char bh = hex_to_dec(buf[stri++]);
			signed char bl = hex_to_dec(buf[stri++]);
			if (bh != -1 && bl != -1)
			{
				colorByteSet[i] = bl | (bh << 4);
			}
		}
	}
	return color;
}