#include "..\..\Common.h"

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define IDSIZE     16               // ä�� ID(�յ� <> ��ȣ ����): ���� ����
#define MSGSIZE    240              // ä�� �޽���: ���� ����
#define BUFSIZE    (IDSIZE+MSGSIZE) // ���� ��ü ���� = ä�� ID + ä�� �޽���

int main(int argc, char *argv[])
{
	int retval;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <IP> <ChatID>\n", argv[0]);
		exit(1);
	}

	// ����� �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];

	// ����� �μ��� ���޵� ä�� ID�� �����ϵ� ���̸� �����Ѵ�.
	const char *chatid = argv[2];
	if (strlen(chatid) > (IDSIZE - 2)) { // �յ� <> ��ȣ�� ���� ���̸� ����Ѵ�.
		fprintf(stderr, "<ChatID>�� �ִ� ���̴� %d�Դϴ�.", (IDSIZE - 2));
		exit(1);
	}

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
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

	// ������ ��ſ� ����� ����
	char msg[MSGSIZE + 1];
	char buf[BUFSIZE + 1];
	int len;

	// ������ ������ ���
	while (1) {
		// �޽��� �Է�
		printf("<%s> ", chatid);
		if (fgets(msg, MSGSIZE + 1, stdin) == NULL)
			break;

		// '\n' ���� ����
		len = (int)strlen(msg);
		if (msg[len - 1] == '\n')
			msg[len - 1] = '\0';
		if (strlen(msg) == 0)
			break;

		// �޽��� ������
		// -- ä�� ID�� �޽����� ���ļ� ���� ���̷� ������.
		sprintf(buf, "<%s> %s", chatid, msg);
		retval = send(sock, buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}

		// �޽��� �ޱ�
		// -- �׻� ���� ���� �����͸� �д´�.
		retval = recv(sock, buf, BUFSIZE, MSG_WAITALL);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}
		else if (retval == 0)
			break;

		// ���� �޽��� ���
		buf[retval] = '\0';
		printf("%s\n", buf);
	}

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
