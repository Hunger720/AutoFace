#include "io.h"

int char2int(const char *c){
	int result = 0;
	string t(c);
	for(int i = 0; i < t.size(); i++){
		result = 10*result + c[i] - '0'; 
	}
	return result;
}

float char2float(const char *c){
	float result = 0, incr = 10, sign = 1;
	string t(c);
	if(c[0] == '-')sign = -1;
	else result = c[0] - '0';
	for(int i = 1; i < t.size(); i++){
		if(c[i] == '.') { incr = 0.1; continue; }
		if(incr > 1)result = result*incr + c[i] - '0';
		else {result += (c[i] - '0')*incr; incr *= 0.1;};
	}
	return result*sign;
}

void char2vertex(char *c, float &x, float &y, float &z){
	x = 0, y = 0, z = 0;
	float sign = 1, step = 10;
	string t(c), s[4];
	int n = t.size(), count = 1;

	for(int i = 0; i < n;){
		if(c[i] != ' '){
			s[0] = c[i];
			s[count].append(s[0]);
			i++;
		}else{
			while(c[i] == ' ')i++;
			count++;
		}
	}

	for(int i = 1; i < 4; i++){
		for(int j = 0; j < s[i].size(); j++){
			switch(s[i].at(j)){
			case '-': sign = -1;
				break;
			case '.':
				break;
			default: step *=0.1;
				if(i == 1)x += step*(s[i].at(j)-'0');
				if(i == 2)y += step*(s[i].at(j)-'0');
				if(i == 3)z += step*(s[i].at(j)-'0');
			}
		}
		if(i == 1)x *= sign;
		if(i == 2)y *= sign;
		if(i == 3)z *= sign;
		sign = 1, step = 10;
	}
}

void char2face(char *face, int &a, int &b, int &c){
	a = 0, b = 0, c = 0;
	string t(face), s[4];
	int n = t.size(), count = 1;

	for(int i = 0; i < n;){
		if(face[i] != ' '){
			s[0] = face[i];
			s[count].append(s[0]);
			i++;
		}else{
			while(face[i] == ' ')i++;
			count++;
		}
	}

	a = char2int(s[1].data());
	b = char2int(s[2].data());
	c = char2int(s[3].data());
}

void char2UnitData(char *c, int &index, float &x, float &y, float&z){
	index = 0, x = 0, y = 0, z = 0;
	string t(c), s[5];
	float sign = 1, step = 10;
	int n = t.size(), count = 1;

	for(int i = 0; i < n;){
		if(c[i] != ' '){
			s[0] = c[i];
			s[count].append(s[0]);
			i++;
		}else{
			while(c[i] == ' ')i++;
			count++;
		}
	}
	index = char2int(s[1].data());

	for(int i = 2; i < 5; i++){
		for(int j = 0; j < s[i].size(); j++){
			switch(s[i].at(j)){
			case '-': sign = -1;
				break;
			case '.':
				break;
			default: step *=0.1;
				if(i == 2)x += step*(s[i].at(j)-'0');
				if(i == 3)y += step*(s[i].at(j)-'0');
				if(i == 4)z += step*(s[i].at(j)-'0');
			}
		}
		if(i == 2)x *= sign;
		if(i == 3)y *= sign;
		if(i == 4)z *= sign;
		sign = 1, step = 10;
	}
}

void char2TexCoord(char *c, float &x, float &y){
	x = 0, y = 0;
	float sign = 1, step = 10;
	string t(c), s[3];
	int count = 1;

	for(int i = 0; i < t.size();){
		if(c[i] != ' '){
			s[0] = c[i];
			s[count].append(s[0]);
			i++;
		}else{
			while(c[i] == ' ')i++;   //消去空格，除非开始读到下一个数据
			count++;
		}
	}

	for(int i = 1; i < 3; i++){
		for(int j = 0; j < s[i].size(); j++){
			switch(s[i].at(j)){
			case '-': sign = -1;
				break;
			case '.':
				break;
			default: step *=0.1;    //ps：增量的计算逻辑有待商榷
				if(i == 1)x += step*(s[i].at(j)-'0');
				if(i == 2)y += step*(s[i].at(j)-'0');
			}
		}
		if(i == 1)x *= sign;
		if(i == 2)y *= sign;
		sign = 1, step = 10;
	}
}