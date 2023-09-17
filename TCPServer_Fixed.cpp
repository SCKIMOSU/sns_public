#include "..\..\Common.h"

#define SERVERPORT 9000
#define IDSIZE     16               // 채팅 ID(앞뒤 <> 기호 포함): 고정 길이
#define MSGSIZE    240              // 채팅 메시지: 고정 길이
#define BUFSIZE    (IDSIZE+MSGSIZE) // 버퍼 전체 길이 = 채팅 ID + 채팅 메시지

int main(int argc, char *argv[])
{
	int retval;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <ChatID>\n", argv[0]);
		exit(1);
	}

	// 명령행 인수로 전달된 채팅 ID를 저장하되 길이를 점검한다.
	const char *chatid = argv[1];
	if (strlen(chatid) > (IDSIZE - 2)) { // 앞뒤 <> 기호를 빼고 길이를 계산한다.
		fprintf(stderr, "<ChatID>의 최대 길이는 %d입니다.", (IDSIZE - 2));
		exit(1);
	}

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char msg[MSGSIZE + 1];
	char buf[BUFSIZE + 1];
	int len;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr *)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {
			// 메시지 받기
			// -- 항상 고정 길이 메시지를 읽는다.
			retval = recv(client_sock, buf, BUFSIZE, MSG_WAITALL);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 메시지 출력
			buf[retval] = '\0';
			printf("%s\n", buf);

			// 메시지 입력
			printf("<%s> ", chatid);
			if (fgets(msg, MSGSIZE + 1, stdin) == NULL)
				break;

			// '\n' 문자 제거
			len = (int)strlen(msg);
			if (msg[len - 1] == '\n')
				msg[len - 1] = '\0';
			if (strlen(msg) == 0)
				break;

			// 메시지 보내기
			// -- 채팅 ID와 메시지를 합쳐서 고정 길이로 보낸다.
			sprintf(buf, "<%s> %s", chatid, msg);
			retval = send(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
