#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//TYPE_NUM must be end of the list
enum TYPE {ARGERROR=-1, ENCODE_CONS, ENCODE_FILE, DECODE_CONS, DECODE_FILE, TYPE_NUM};
//めんどいから実際には環境依存
enum CODE_TYPE{UTF8};
/*
UTF-8 : 3byte per char
*/

void print_usage(void)
{
	printf("USAGE: option[ decode : -d, from file : -f ] input[ console input, file path]\n");
}

int parse(int arg_num, char *arg_str[])
{
	enum CODE_TYPE code_type=UTF8;
	int flag_aug_decode=0;
	int flag_aug_file=0;

	if(arg_num == 2){
		if(arg_str[1][0] == '-' && arg_str[1][1] == 'h' && arg_str[1][2] == '\0') return ARGERROR;
		else return ENCODE_CONS;
	}
	else if(arg_num == 3){
		if(strlen(arg_str[1]) < 2) return ARGERROR;
		if(arg_str[1][0] == '-' && arg_str[1][1] == 'd' && arg_str[1][2] == '\0') return DECODE_CONS;
		else if(arg_str[1][0] == '-' && arg_str[1][1] == 'f' && arg_str[1][2] == '\0') return ENCODE_FILE;
		else return ARGERROR;
	}
	else if(arg_num == 4){
		int i;
		for(i=1; i < 3; i++){
			if(strlen(arg_str[i]) < 2) return ARGERROR;

			if(arg_str[i][0] == '-' && arg_str[i][1] == 'd' && arg_str[i][2] == '\0') flag_aug_decode = 1;
			else if(arg_str[i][0] == '-' && arg_str[i][1] == 'f' && arg_str[i][2] == '\0') flag_aug_file = 1;
			//else if(...) //code_type
			else return ARGERROR;
		}

		if(flag_aug_decode){
			if(flag_aug_file) return DECODE_FILE + code_type*TYPE_NUM;
			else return DECODE_CONS + code_type*TYPE_NUM;
		}
		else return ENCODE_FILE + code_type*TYPE_NUM;
	}
	else return ARGERROR;

}

//配列の要素チェックはしてないので自分で考えて！！！
void encode(char *dist, int c, char **mau){
	char bits[8];
	int lev;

	for(lev=7; lev >= 0; lev--){
		bits[lev] = c & 0x01;
		c = c >> 1;
	}

	sprintf(dist, "%s%s%s%s%s%s%s%s", mau[bits[0]], mau[bits[1]], mau[bits[2]], mau[bits[3]], mau[bits[4]], mau[bits[5]], mau[bits[6]], mau[bits[7]]);
	//printf("%s%s%s%s%s%s%s%s", mau[bits[0]], mau[bits[1]], mau[bits[2]], mau[bits[3]], mau[bits[4]], mau[bits[5]], mau[bits[6]], mau[bits[7]]);

	return ;
}

// strには８文字(8 byte)の文字列が来ていることを期待している。
void decode(char *dist, char *str, char *code_ma, char *code_u)
{
	//「ま」と「う」でコードのバイト数が同じという仮定をしている。
	int len_code = strlen(code_ma);
	int str_len = strlen(str);
	int i,j;
	int bit_count;

	int ma, u;

	*dist = 0;

	if((str_len%(8*len_code))){
		printf("decode : string num error.\n");
		return ;
	}

	for(i = 0, bit_count=7; i < 8*len_code; i +=j){
		for(j = 0, ma=0, u=0; j < len_code; j++){
			ma += str[i+j] - code_ma[j];
			//u +=  str[i+j+k] - code_u[k]; //文字コードが一致してれば、片方の比較で済む。
		}
		*dist += (1 << bit_count) * (ma == 0 ? 0 : 1);
		bit_count--;
	}
}

int utf_char_byte_len(char c)
{
	/*
	0x00 -> 0x7f : start of 1byte
	0x80 -> 0xbf : code of multibyte
	0xc0 -> 0xdf : start of 2bytes char
	0xe0 -> 0xef : start of 3bytes char
	0xf0 -> 0xff : start of 4bytes char
	バイトごとに別々に分けて出力するとうまくいかない
	%s だと途中でぶった切られてもうまくいくようだが　%c　でバイトごとはどうやら個別に解釈されるので
	ASCIIの謎の文字になるっぽい。

	処理系依存になりそうだが、引き算で求めることにする。
	入力にミスがなければ0x80 -> 0xbfは出てこないはず。
	*/
	if(c-0xf0 >= 0) return 4;
	else if(c-0xe0 >= 0) return 3;
	else if(c-0xc0 >= 0) return 2;
	else return 1;
}

int main(int argc, char *argv[])
{
	char input;
	int case_num;
	int input_len;

	char *mau[2];
	char ma[] = "ま"; // UTF-8 = [ -29, -127, -66]
	char u[] = "う"; // UTF-8 = [ -29, -127, -122]

	FILE *fp;
	
	int i,j,k;
	int char_byte;

	char *encoded;
	char *decoded;
	char c;

	mau[0] = ma;
	mau[1] = u;

	case_num = parse(argc, argv);
	input_len = strlen(argv[argc-1]);

	// これ以上増えそうなら関数にして、バイトだけ渡すようにすべきかもしれんな。
	switch(case_num){
		case ENCODE_CONS+UTF8*TYPE_NUM:
			encoded = malloc(sizeof(char)*8*3+1);
			for(i=0; i < input_len; i++){
				encode(encoded, argv[argc-1][i], mau);
				printf("%s", encoded);
			}
			break;

		case ENCODE_FILE+UTF8*TYPE_NUM:
			encoded = malloc(sizeof(char)*8*3+1);
			fp = fopen(argv[argc-1], "r");
			while((c = fgetc(fp)) != EOF){
				encode(encoded, c, mau);
				printf("%s", encoded);
			}
			break;

		case DECODE_CONS+UTF8*TYPE_NUM:
			// ここ頭悪そうなコードブロック
			encoded = malloc(sizeof(char)*8*3+1);
			encoded[8*3] = '\0';
			decoded = malloc(sizeof(char)*4+1);

			for(i=0; i < input_len;){
				for(j=0; j < 8*3; j++){
					encoded[j] = argv[argc-1][i+j];
				}

				decode(&c, encoded, ma, u);
				decoded[0] = c;
				char_byte = utf_char_byte_len(c);

				for(k=1, i+=j; k < char_byte; k++,i+=j){
					for(j=0; j < 8*3; j++){
						encoded[j] = argv[argc-1][i+j];
					}
					decode(&c, encoded, ma, u);
					decoded[k] = c;
				}
				decoded[k] = '\0';
				printf("%s", decoded);
			}
			break;

		case DECODE_FILE+UTF8*TYPE_NUM:
			encoded = malloc(sizeof(char)*8*3+1);
			decoded = malloc(sizeof(char)*4+1);

			fp = fopen(argv[argc-1], "r");
			while((fgets(encoded, 8*3+1, fp)) != NULL){
				decode(&c, encoded, ma, u);
				decoded[0] = c;
				char_byte = utf_char_byte_len(c);

				for(i=1; i < char_byte; i++){
					fgets(encoded, 8*3, fp);
					decode(&c, encoded, ma, u);
					decoded[i] = c;
				}
				encoded[i] = '\0'; 
				printf("%s", decoded);
			}

			break;

		case ARGERROR:
			print_usage();
			break;
	}

	return 0;
}