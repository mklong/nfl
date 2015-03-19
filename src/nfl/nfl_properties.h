#ifndef __NFL_PROPERTIES_H__
#define __NFL_PROPERTIES_H__
/** 
 * @file	nfl_properties.h
 * @brief	
 * 
 * detail...
 * 
 * @author	mklong
 * @version	1.0
 * @date	2014/2/23
 * 
 * @see		
 * 
 * <b>History:</b><br>
 * <table>
 *  <tr> <th>Version	<th>Date		<th>Author	<th>Notes</tr>
 *  <tr> <td>1.0		<td>2014/2/23	<td>mklong	<td>Create this file</tr>
 * </table>
 * 
 */

namespace nfl
{

struct acceptor_properties
{
	int backlog;
	int rcvbuf;
	int sndbuf;
};

struct socket_properties
{
	//setsockopt 
	bool tcp_nodelay;

	//timeout
	struct timeval connect_timeout;
	struct timeval read_timeout;
	struct timeval write_timeout;
};





}

#endif /* __NFL_PROPERTIES_H__ */
