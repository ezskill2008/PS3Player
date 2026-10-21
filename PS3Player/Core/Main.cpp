#include <libPs3Decrypt/Include/DecryptISO.h>

#pragma comment(lib, "libPs3Core.lib")
#pragma comment(lib, "libPs3Decrypt.lib")

#include <Windows.h>

int main() {
#if _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	CryptKey isoKey{};
	if (!LpsReadKey(
		"D:\\code\\PS3Content\\Minerva_Myrient\\Redump\\Sony - PlayStation 3 - Disc Keys\\Angry Birds Trilogy (Europe) (En,Fr,De,Es,It).key",
		isoKey))
		return -1;

	IsoDisk disk{};
	if (!LpsLoadDisk(
		"D:\\code\\PS3Content\\Minerva_Myrient\\Redump\\Sony - PlayStation 3\\Angry Birds Trilogy (Europe) (En,Fr,De,Es,It).iso",
		disk))
		return -1;;

	return 0;
}
