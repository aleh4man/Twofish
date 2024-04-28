#include "bitmap/bitmap_image.hpp"
#include <ctime>
#include <cmath>
#include <iostream>

unsigned char MDS[4][4];
unsigned char t0q0[] = { 0x8, 0x1, 0x7, 0xD, 0x6, 0xF, 0x3, 0x2, 0x0, 0xB, 0x5, 0x9, 0xE, 0xC, 0xA, 0x4 };
unsigned char t1q0[] = { 0xE, 0xC, 0xB, 0x8, 0x1, 0x2, 0x3, 0x5, 0xF, 0x4, 0xA, 0x6, 0x7, 0x0, 0x9, 0xD };
unsigned char t2q0[] = { 0xB, 0xA, 0x5, 0xE, 0x6, 0xD, 0x9, 0x0, 0xC, 0x8, 0xF, 0x3, 0x2, 0x4, 0x7, 0x1 };
unsigned char t3q0[] = { 0xD, 0x7, 0xF, 0x4, 0x1, 0x2, 0x6, 0xE, 0x9, 0xB, 0x3, 0x0, 0x8, 0x5, 0xC, 0xA };
unsigned char t0q1[] = { 0x2, 0x8, 0xB, 0xD, 0xF, 0x7, 0x6, 0xE, 0x3, 0x1, 0x9, 0x4, 0x0, 0xA, 0xC, 0x5 };
unsigned char t1q1[] = { 0x1, 0xE, 0x2, 0xB, 0x4, 0xC, 0x3, 0x7, 0x6, 0xD, 0xA, 0x5, 0xF, 0x9, 0x0, 0x8 };
unsigned char t2q1[] = { 0x4, 0xC, 0x7, 0x5, 0x1, 0x6, 0x9, 0xA, 0x0, 0xE, 0xD, 0x8, 0x2, 0xB, 0x3, 0xF };
unsigned char t3q1[] = { 0xB, 0x9, 0x5, 0x1, 0xC, 0x3, 0xD, 0xE, 0x6, 0x4, 0x7, 0xF, 0x2, 0x0, 0x8, 0xA };

unsigned char RS[4][8];

unsigned long long nulls = 0;
unsigned long long ones = 0;


void make_q(unsigned char* byte, bool num) {
	unsigned char a = *byte & 15;
	unsigned char b = (*byte & (15 << 4)) >> 4;
	
	a = a / 16;
	b = b % 16;

	unsigned char a1 = a ^ b;
	unsigned char b1 = (a ^ (b << 4) ^ (a * 8)) % 16;

	if (!num) {
		a = t0q0[a1];
		b = t1q0[b1];
	}
	else {
		a = t0q1[a1];
		b = t1q1[b1];
	}

	a1 = a ^ b;
	b1 = (a ^ (b << 4) ^ (a * 8)) % 16;

	if (!num) {
		a = t2q0[a1];
		b = t3q0[b1];
	}
	else {
		a = t2q1[a1];
		b = t3q1[b1];
	}

	*byte = (16 * b) + a;
}

void generate_keysK(unsigned char* key, unsigned char** keysK) {
	unsigned char mO[2][4];
	unsigned char mE[2][4];
	short tmp = 0;
	for (short j = 0; j < 4; j++) {
		mE[0][j] = key[3-j];
		mE[1][j] = key[11 - j];
		mO[0][j] = key[7 - j];
		mO[1][j] = key[15 - j];
	}

	for (short i = 0; i < 20; i++) {
		unsigned char bytesE[4] = { 2 * i, 2 * i, 2 * i, 2 * i};
		unsigned char bytesO[4] = { 2 * i + 1, 2 * i + 1, 2 * i + 1, 2 * i + 1};

		make_q(&bytesE[0], false);
		make_q(&bytesE[1], true);
		make_q(&bytesE[2], false);
		make_q(&bytesE[3], true);

		make_q(&bytesO[0], false);
		make_q(&bytesO[1], true);
		make_q(&bytesO[2], false);
		make_q(&bytesO[3], true);

		for (short j = 0; j < 4; j++) {
			bytesE[j] = bytesE[j] ^ mE[1][j];
			bytesO[j] = bytesO[j] ^ mO[1][j];
		}

		make_q(&bytesE[0], false);
		make_q(&bytesE[1], false);
		make_q(&bytesE[2], true);
		make_q(&bytesE[3], true);

		make_q(&bytesO[0], false);
		make_q(&bytesO[1], false);
		make_q(&bytesO[2], true);
		make_q(&bytesO[3], true);

		for (short j = 0; j < 4; j++) {
			bytesE[j] = bytesE[j] ^ mE[0][j];
			bytesO[j] = bytesO[j] ^ mO[0][j];
		}
		
		make_q(&bytesE[0], true);
		make_q(&bytesE[1], false);
		make_q(&bytesE[2], true);
		make_q(&bytesE[3], false);

		make_q(&bytesO[0], true);
		make_q(&bytesO[1], false);
		make_q(&bytesO[2], true);
		make_q(&bytesO[3], false);
		
		for (short j = 0; j < 4; j++) {
			bytesE[j] = bytesE[0] * MDS[j][0] + bytesE[1] * MDS[j][1] + bytesE[2] * MDS[j][2] + bytesE[3] * MDS[j][3];
		}
		
		for (short j = 0; j < 4; j++) {
			bytesO[j] = bytesO[0] * MDS[j][0] + bytesO[1] * MDS[j][1] + bytesO[2] * MDS[j][2] + bytesO[3] * MDS[j][3];
		}

		unsigned char tmp = bytesO[0];
		for (short j = 1; j < 4; j++) {
			bytesO[j - 1] = bytesO[j];
		}
		bytesO[3] = tmp;

		unsigned long long mod = 1;
		mod = mod << 32;
		unsigned int a1 = 0;
		unsigned int a2 = 0;
		unsigned int a3 = 0;
		a1 = a1 | bytesE[0];
		a2 = a2 | bytesO[0];
		for (short j = 1; j < 4; j++) {
			a1 = (a1 << 8) | bytesE[j];
			a2 = (a2 << 8) | bytesO[j];
		}

		a3 = ((unsigned long long)a1 + a2) % mod;

		for (short j = 3; j >= 0; j--) {
			bytesE[j] = (a3 & ((unsigned long)255 << (8 * (3-j)))) >> (8 * (3 - j));
		}

		a2 = a3 = (a1 + (unsigned long long)2*a2) % mod;
		a1 = 0;
		a1 = (a2 & ((unsigned long long)512 << 23)) >> 23;
		a2 = (a2 << 9) | a1;
		for (short j = 3; j >= 0; j--) {
			bytesO[j] = (a2 & ((unsigned long long)255 << (8 * (3 - j)))) >> (8 * (3 - j));
		}
		for (short j = 3; j >= 0; j--) {
			keysK[i * 2][j] = bytesE[j];
			keysK[i * 2 + 1][j] = bytesO[j];
		}
	}
}

void generate_keysS(unsigned char* key, unsigned char** keysS) {
	RS[0][0] = 0x01; RS[0][1] = 0xA4; RS[0][2] = 0x55; RS[0][3] = 0x87; RS[0][4] = 0x5A; RS[0][5] = 0x58; RS[0][6] = 0xDB; RS[0][7] = 0x9E;
	RS[1][0] = 0xA4; RS[1][1] = 0x56; RS[1][2] = 0x82; RS[1][3] = 0xF3; RS[1][4] = 0x1E; RS[1][5] = 0xC6; RS[1][6] = 0x68; RS[1][7] = 0xE5;
	RS[2][0] = 0x02; RS[2][1] = 0xA1; RS[2][2] = 0xFC; RS[2][3] = 0xC1; RS[2][4] = 0x47; RS[2][5] = 0xAE; RS[2][6] = 0x3D; RS[2][7] = 0x19;
	RS[3][0] = 0xA4; RS[3][1] = 0x55; RS[3][2] = 0x87; RS[3][3] = 0x5A; RS[3][4] = 0x58; RS[3][5] = 0xDB; RS[3][6] = 0x9E; RS[3][7] = 0x03;

	for (short j = 0; j < 4; j++) {
		keysS[0][j] = key[0] * RS[j][0] + key[1] * RS[j][1] + key[2] * RS[j][2] + key[3] * RS[j][3] + key[4] * RS[j][4] + key[5] * RS[j][5] + key[6] * RS[j][6] + key[7] * RS[j][7];
		keysS[1][j] = key[8] * RS[j][0] + key[9] * RS[j][1] + key[10] * RS[j][2] + key[11] * RS[j][3] + key[12] * RS[j][4] + key[13] * RS[j][5] + key[14] * RS[j][6] + key[15] * RS[j][7];
	}
}

void g_function(unsigned char* bytes1, unsigned char* bytes2, unsigned char** keysS) {
	make_q(&bytes1[0], false);
	make_q(&bytes1[1], true);
	make_q(&bytes1[2], false);
	make_q(&bytes1[3], true);

	make_q(&bytes2[0], false);
	make_q(&bytes2[1], true);
	make_q(&bytes2[2], false);
	make_q(&bytes2[3], true);

	for (short j = 0; j < 4; j++) {
		bytes1[j] = bytes1[j] ^ keysS[0][j];
		bytes2[j] = bytes2[j] ^ keysS[0][j];
	}

	make_q(&bytes1[0], false);
	make_q(&bytes1[1], false);
	make_q(&bytes1[2], true);
	make_q(&bytes1[3], true);

	make_q(&bytes2[0], false);
	make_q(&bytes2[1], false);
	make_q(&bytes2[2], true);
	make_q(&bytes2[3], true);

	for (short j = 0; j < 4; j++) {
		bytes1[j] = bytes1[j] ^ keysS[1][j];
		bytes2[j] = bytes2[j] ^ keysS[1][j];
	}

	make_q(&bytes1[0], true);
	make_q(&bytes1[1], false);
	make_q(&bytes1[2], true);
	make_q(&bytes1[3], false);

	make_q(&bytes2[0], true);
	make_q(&bytes2[1], false);
	make_q(&bytes2[2], true);
	make_q(&bytes2[3], false);

	for (short j = 0; j < 4; j++) {
		bytes1[j] = bytes1[0] * MDS[j][0] + bytes1[1] * MDS[j][1] + bytes1[2] * MDS[j][2] + bytes1[3] * MDS[j][3];
	}

	for (short j = 0; j < 4; j++) {
		bytes2[j] = bytes2[0] * MDS[j][0] + bytes2[1] * MDS[j][1] + bytes2[2] * MDS[j][2] + bytes2[3] * MDS[j][3];
	}
}

void make_twofish(unsigned char* iv, unsigned char* key) {
	unsigned char** keysK = new unsigned char* [40];
	unsigned char** keysS = new unsigned char* [2];
	for (short i = 0; i < 40; i++) {
		keysK[i] = new unsigned char[4];
		if (i < 2) keysS[i] = new unsigned char[4];
	}

	generate_keysK(key, keysK);
	//std::cout << "\n\n\nresult:\n";
	//for (short i = 0; i < 40; i++) {
	//	std::cout << i << ": ";
	//	for (short j = 0; j < 4; j++) {
	//		std::cout << (int)keysK[i][j] << " ";
	//	}
	//	std::cout << "\n";
	//}

	//отбеливание
	unsigned char b[4][4];
	for (short i = 0; i < 4; i++) {
		for (short j = 0; j < 4; j++) {
			iv[i * 4 + j] = b[i][j] = iv[i * 4 + j] ^ keysK[i][j];
		}
	}

	//ебучий цикл раундов
	for (short i = 0; i < 16; i++) {
		unsigned char byteSC[4];
		char tmp = b[1][0];
		for (short i = 1; i < 4; i++) {
			byteSC[i - 1] = b[1][i];
		}
		byteSC[3] = tmp;

		g_function(b[0], byteSC, keysS);

		//сложение по модулю 2^32
		unsigned int a1 = 0;
		unsigned int a2 = 0;
		unsigned int a3 = 0;
		unsigned int a4 = 0;
		a1 = a1 | b[0][0];
		a2 = a2 | byteSC[0];
		for (short j = 1; j < 4; j++) {
			a1 = (a1 << 8) | b[0][j];
			a2 = (a2 << 8) | byteSC[j];
		}
		unsigned long long mod = 1;
		mod = mod << 32;
		a3 = ((unsigned long long)a1 + a2) % mod;
		a2 = (a1 + (unsigned long long)2 * a2) % mod;
		
		//сложение по модулю 2^32 с ключами
		a1 = a1 | keysK[2 * i + 8][0];
		a4 = a4 | keysK[2 * i + 9][0];
		for (short j = 1; j < 4; j++) {
			a1 = (a1 << 8) | keysK[2 * i + 8][j];
			a4 = (a4 << 8) | keysK[2 * i + 9][j];
		}
		a3 = ((unsigned long long)a3 + a1) % mod;
		a2 = ((unsigned long long)a2 + a4) % mod;

		//ксор с байтами
		a1 = a1 | b[2][0];
		a4 = a4 | b[3][0];
		for (short j = 1; j < 4; j++) {
			a1 = (a1 << 8) | b[2][j];
			a4 = (a4 << 8) | b[3][j];
		}

		a3 = a3 ^ a1;
		unsigned long long cif = a3 & 1;
		a3 = a3 >> 1;
		a3 = a3 | (cif << 31);

		cif = (((unsigned long long)1 << 31) & a4) >> 31;
		a4 = a4 << 1;
		a4 = a4 | cif;
		a2 = a2 ^ a4;

		//перестановка
		for (short j = 0; j < 4; j++) {
			b[2][j] = b[0][j];
			b[3][j] = b[1][j];
		}

		for (short j = 3; j >= 0; j--) {
			b[0][j] = (a3 & ((unsigned long)255 << (8 * (3 - j)))) >> (8 * (3 - j));
			b[1][j] = (a2 & ((unsigned long)255 << (8 * (3 - j)))) >> (8 * (3 - j));
		}
	}

	for (short i = 0; i < 4; i++) {
		for (short j = 0; j < 4; j++) {
			iv[i * 4 + j] = b[i][j] = iv[i * 4 + j] ^ keysK[i+4][j];
		}
	}
}



int main() {
	std::string file, key, status;
	std::cout << "enter filename: ";
	std::cin >> file;
	do {
		std::cout << "encrypt or decrypt: ";
		std::cin >> status;
	} while ((status != std::string("decrypt")) && (status != std::string("encrypt")));

	unsigned char keyG[16];
	unsigned char* iv = new unsigned char[16];
	unsigned char* iv_begin = new unsigned char[16];
	if (status != std::string("decrypt")) {
		std::cout << "enter key: ";
		std::cin >> key;
		if (key.size() < 16) {
			if (key == std::string("auto")) {
				for (short i = 0; i < 16; i++) {
					srand(time(NULL));
					keyG[i] = rand() % 255;
				}
			}
			else return -1;
		}
		else {
			const char* str = key.c_str();
			for (short i = 0; i < 16; i++) { keyG[i] = str[i]; }
		}
		for (int i = 0; i < 16; i++) {
			srand(time(NULL));
			iv[i] = iv_begin[i] = rand() % 255;
		}
	}
	else {
		std::ifstream data("id.tfd", std::ios::binary);
		std::string tmp;
		data >> tmp;
		for (short i = 0; i < 16; i++) {
			keyG[i] = tmp[i];
		}
		for (int i = 16; i < 32; i++) {
			iv[i % 16] = tmp[i];
		}

	}
	bitmap_image pic(file);

	if (!pic) {
		return -1;
	}

	bitmap_image pic_crypt(pic.width(), pic.height());

	//!удалить
	time_t start = time(NULL);
	//!удалить

	//load picture
	unsigned long long bytes = (unsigned long long)pic.width() * pic.height() * 3;
	unsigned long long vec_height = bytes / 16;
	unsigned short excess = bytes % 16;
	if (excess > 0) {
		vec_height++;
	}
	unsigned char** vec16 = new unsigned char* [vec_height];
	for (unsigned long long i = 0; i < vec_height; i++) {
		vec16[i] = new unsigned char[16];
	}
	if (excess > 0) {
		for (int i = excess; i < 16; i++) vec16[vec_height - 1][i] = 0;
	}
	rgb_t pixel;

	for (unsigned int y = 0; y < pic.height(); y++) {
		for (unsigned int x = 0; x < pic.width(); x++) {
			pixel = pic.get_pixel(x, y);
			unsigned long coord = (y * pic.width() + x) * 3;
			for (short t = 0; t < 3; t++) {
				switch (t) {
				case 0:
					vec16[(coord + t) / 16][(coord + t) % 16] = pixel.red;
					break;
				case 1:
					vec16[(coord + t) / 16][(coord + t) % 16] = pixel.green;
					break;
				case 2:
					vec16[(coord + t) / 16][(coord + t) % 16] = pixel.blue;
					break;
				default:
					break;
				}
			}
		}
	}
	pic.clear();

	MDS[0][0] = 0x01; MDS[0][1] = 0xEF; MDS[0][2] = 0x5B; MDS[0][3] = 0x5B;
	MDS[1][0] = 0x5B; MDS[1][1] = 0xEF; MDS[1][2] = 0xEF; MDS[1][3] = 0x01;
	MDS[2][0] = 0xEF; MDS[2][0] = 0x5B; MDS[2][0] = 0x01; MDS[2][0] = 0xEF;
	MDS[3][0] = 0xEF; MDS[3][0] = 0x01; MDS[3][0] = 0xEF; MDS[3][0] = 0x5B;

	//шифрование
	//генерация вектора инициализации

	//цикл CFB
	for (short j = 0; j < 16; j++) {
		keyG[j] = '1';
		//if (j%2 == 0) iv[j] = '1';
		//else iv[j] = 'a';
	}
	unsigned char* bits = new unsigned char[bytes * 8];
	if (status != std::string("decrypt")) {
		for (unsigned long long i = 0; i < vec_height; i++) {
			for (short j = 9; j < 16; j+=2) {
				iv[j] = ~iv[j];
			}
			make_twofish(iv, keyG);
			for (short j = 0; j < 16; j++) {
				iv[j] = vec16[i][j] = iv[j] ^ vec16[i][j];
				for (int short k = 0; k < 8; k++) {
					char bit = (vec16[i][j] & (1 << k)) >> k;
					if (bit == 1) ones++;
					else if (bit == 0) nulls++;
					else {
						std::cout << "ERROR";
						return -1;
					}
					bits[(i * 16 + j) * 8 + k] = bit;
				}
			}
		}
	}
	else {
		for (unsigned long long i = 0; i < vec_height; i++) {
			make_twofish(iv, keyG);
			char tmp[16];
			for (short j = 0; j < 16; j++) {
				tmp[j] = vec16[i][j];
			}
			for (short j = 0; j < 16; j++) {
				vec16[i][j] = iv[j] ^ vec16[i][j];
				iv[j] = tmp[j];
			}
		}
	}











	std::cout << "\nfreq test: " << ((nulls - ones) * (nulls - ones) / (double)(bytes * 8)) << "\n";

	int A = 0;
	unsigned long long n = bytes * 8;
	unsigned long long d = n / 2-3;
	for (unsigned long long i = 0; i < (n - d - 1); i++) {
		char x = bits[i];
		char y = bits[i + d];
		A += x ^ y;
	}
	std::cout << A << std::endl;
	std::cout << "\nautocorrelation: " << (2 * (A - (double)(n - d) / 2) / sqrt(n - d)) << "\n";

	//int i1 = 1;
	//int k = 0;
	//double* ei = new double[n];
	//double etmp;
	//do {
	//	etmp = (n - i1 + 3) / pow(2, i1 + 2);
	//	ei[i1++] = etmp;
	//} while (etmp >= 5);
	//k = i1;
	//int counter = 0;
	//int* Bi = new int[n];
	//int* Gi = new int[n];

	//while (i1 != 0) {
	//	Bi[i1] = 0;
	//	Gi[i1] = 0;

	//	for (int j = 1; j < n; j++) {
	//		if (bits[j] == bits[j - 1]) {
	//			counter++;
	//		}
	//		else {
	//			if (counter == i1) {
	//				if (bits[j - 1] == 1) Gi[i1]++;
	//				else Bi[i1]++;
	//			}
	//			counter = 0;
	//		}
	//	}
	//	i1--;
	//}
	//double sumB = 0;
	//double sumG = 0;
	//while (k != 0) {
	//	if (ei[k] != 0) {
	//		sumB += pow((Bi[k] - ei[k]), 2) / ei[k];
	//		sumG += pow((Gi[k] - ei[k]), 2) / ei[k];
	//	}
	//	k--;
	//}

	//std::cout << "\nseries: " << (double)(sumB + sumG) << "\n";
		
		
		
		
		
		
	//запись файла
	for (unsigned int y = 0; y < pic_crypt.height(); y++) {
		for (unsigned int x = 0; x < pic_crypt.width(); x++) {
			unsigned long coord = (y * pic_crypt.width() + x) * 3;
			for (short t = 0; t < 3; t++) {
				switch (t) {
				case 0:
					pixel.red = vec16[(coord + t) / 16][(coord + t) % 16];
					break;
				case 1:
					pixel.green = vec16[(coord + t) / 16][(coord + t) % 16];
					break;
				case 2:
					pixel.blue = vec16[(coord + t) / 16][(coord + t) % 16];
					break;
				default:
					break;
				}
			}
			pic_crypt.set_pixel(x, y, pixel);
		}
	}

	//!удалить
	time_t end = time(NULL) - start;
	if ((end % 60) < 10) std::cout << "time: " << end / 60 << ":0" << end % 60 << "\n";
	else std::cout << "time: " << end / 60 << ":" << end % 60 << "\n";
	//!удалить
	if (status != std::string("decrypt")) {
		std::ofstream data("id.tfd", std::ios::binary);
		if (key == std::string("auto")) {
			for (short j = 0; j < 16; j++) {
				data << keyG[j];
			}
		}
		else {
			data << key;
		}

		data << ":";
		for (short j = 0; j < 16; j++) {
			data << iv_begin[j];
		}
		data.close();
		pic_crypt.save_image("res.bmp");
	}
	else pic_crypt.save_image("res_dec.bmp");
	printf("\a");
	return 0;
}