#include "..\..\Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define IDSIZE     16               // 채팅 ID(앞뒤 <> 기호 포함): 고정 길이
#define MSGSIZE    240              // 채팅 메시지: 고정 길이
#define BUFSIZE    (IDSIZE+MSGSIZE) // 버퍼 전체 길이 = 채팅 ID + 채팅 메시지

int main(int argc, char *argv[])
{
	int retval;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <IP> <ChatID>\n", argv[0]);
		exit(1);
	}

	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];

	// 명령행 인수로 전달된 채팅 ID를 저장하되 길이를 점검한다.
	const char *chatid = argv[2];
	if (strlen(chatid) > (IDSIZE - 2)) { // 앞뒤 <> 기호를 빼고 길이를 계산한다.
		fprintf(stderr, "<ChatID>의 최대 길이는 %d입니다.", (IDSIZE - 2));
		exit(1);
	}

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// 데이터 통신에 사용할 변수
	char msg[MSGSIZE + 1];
	char buf[BUFSIZE + 1];
	int len;

	// 서버와 데이터 통신
	while (1) {
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
		retval = send(sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// 메시지 받기
		// -- 항상 고정 길이 데이터를 읽는다.
		retval = recv(sock, buf, BUFSIZE, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// 받은 메시지 출력
		buf[retval] = '\0';
		printf("%s\n", buf);
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
