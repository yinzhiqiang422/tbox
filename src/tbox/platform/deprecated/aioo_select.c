/*!The Treasure Box Library
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Copyright (C) 2009 - 2017, TBOOX Open Source Group.
 *
 * @author      ruki
 * @file        aioo_select.c
 *
 */
/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#ifdef TB_CONFIG_OS_WINDOWS
#   include "../windows/interface/interface.h"
#else
#   include <sys/select.h>
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * macros
 */

// FD_ISSET
#ifdef TB_CONFIG_OS_WINDOWS
#   undef FD_ISSET
#   define FD_ISSET(fd, set) tb_ws2_32()->__WSAFDIsSet((SOCKET)(fd), (fd_set FAR *)(set))
#endif

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
static tb_long_t tb_aioo_rtor_select_wait(tb_socket_ref_t sock, tb_size_t code, tb_long_t timeout)
{
    // check
    tb_assert_and_check_return_val(sock, -1);

    // fd
    tb_long_t fd = tb_sock2fd(sock);
    tb_assert_and_check_return_val(fd >= 0, -1);
    
    // init time
    struct timeval t = {0};
    if (timeout > 0)
    {
#ifdef TB_CONFIG_OS_WINDOWS
        t.tv_sec = (LONG)(timeout / 1000);
#else
        t.tv_sec = (timeout / 1000);
#endif
        t.tv_usec = (timeout % 1000) * 1000;
    }

    // init fds
    fd_set  rfds;
    fd_set  wfds;
    fd_set* prfds = (code & TB_AIOE_CODE_RECV || code & TB_AIOE_CODE_ACPT)? &rfds : tb_null;
    fd_set* pwfds = (code & TB_AIOE_CODE_SEND || code & TB_AIOE_CODE_CONN)? &wfds : tb_null;

    if (prfds)
    {
        FD_ZERO(prfds);
        FD_SET(fd, prfds);
    }

    if (pwfds)
    {
        FD_ZERO(pwfds);
        FD_SET(fd, pwfds);
    }
   
    // select
#ifdef TB_CONFIG_OS_WINDOWS
    tb_long_t r = tb_ws2_32()->select((tb_int_t)fd + 1, prfds, pwfds, tb_null, timeout >= 0? &t : tb_null);
#else
    tb_long_t r = select(fd + 1, prfds, pwfds, tb_null, timeout >= 0? &t : tb_null);
#endif
    tb_assert_and_check_return_val(r >= 0, -1);

    // timeout?
    tb_check_return_val(r, 0);

    // error?
    tb_int_t o = 0;
#ifdef TB_CONFIG_OS_WINDOWS
    tb_int_t n = sizeof(tb_int_t);
    tb_ws2_32()->getsockopt(fd, SOL_SOCKET, SO_ERROR, (tb_char_t*)&o, &n);
#else
    socklen_t n = sizeof(socklen_t);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, (tb_char_t*)&o, &n);
#endif
    if (o) return -1;

    // ok
    tb_long_t e = 0;
    if (prfds && FD_ISSET(fd, &rfds)) 
    {
        e |= TB_AIOE_CODE_RECV;
        if (code & TB_AIOE_CODE_ACPT) e |= TB_AIOE_CODE_ACPT;
    }
    if (pwfds && FD_ISSET(fd, &wfds)) 
    {
        e |= TB_AIOE_CODE_SEND;
        if (code & TB_AIOE_CODE_CONN) e |= TB_AIOE_CODE_CONN;
    }
    return e;
}

