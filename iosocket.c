
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include<netinet/in.h>
#include <unistd.h>
#include <string.h>

int openServerSocket()
{

	 struct sockaddr_in my_addr;
	 int listnum = 10;int on=1;
	 int serverFd;
	 
	 memset(&my_addr, 0,sizeof(my_addr));
	
	 if((serverFd = socket(PF_INET,SOCK_STREAM,0))== -1) return -1;
	
	 setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));
	 my_addr.sin_family=PF_INET;

	 if(bind(serverFd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr)) == -1)
	 {
		 if(serverFd!=-1) { close(serverFd); serverFd=-1;}
		 return -1;
	 }
	 if(listen(serverFd,listnum) == -1)
	 {
		 if(serverFd!=-1) { close(serverFd); serverFd=-1;}
		 return -1;
	 }

	
}

void gw_accept(sockfd)
{

}

