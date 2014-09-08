/*
 * Copyright (C) 2014 Ron Pedde (ron@pedde.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#define DBG_FATAL 0
#define DBG_ERROR 1
#define DBG_WARN  2
#define DBG_INFO  3
#define DBG_DEBUG 4

#define X_EMIT(label, fmt, args...) debug_printf(label, fmt, ##args)

#if defined(NDEBUG)
#define DEBUG(format, args...)
#define INFO(format, args...)
#define WARN(format, args...)
#define ERROR(format, args...) X_EMIT(DBG_ERROR, "Error: " format "\n", ##args)
#define FATAL(format, args...) X_EMIT(DBG_FATAL, "Fatal: " format "\n", ##args)
#else
#define DEBUG(format, args...) X_EMIT(DBG_DEBUG, "[DEBUG] %s:%d (%s): " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define INFO(format, args...) debug_printf(DBG_INFO, "[INFO] %s:%d (%s): " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define WARN(format, args...) debug_printf(DBG_WARN, "[WARN] %s:%d (%s): " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define ERROR(format, args...) debug_printf(DBG_ERROR, "[ERROR] %s:%d (%s): " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#define FATAL(format, args...) debug_printf(DBG_FATAL, "[FATAL] %s:%d (%s): " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args)
#endif /* NDEBUG */

extern void debug_set_level(int newlevel);
extern void debug_printf(int level, char *format, ...);

#endif /* _DEBUG_H_ */
