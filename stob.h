static unsigned char number(unsigned char f,unsigned char s){
    unsigned char R;
    R=0x00;
    switch (f){
	case '1':R=0x10;
		break;
	case '2':R=0x20;
		break;
	case '3':R=0x30;
		break;
	case '4':R=0x40;
		break;
	case '5':R=0x50;
		break;
	case '6':R=0x60;
		break;
	case '7':R=0x70;
		break;
	case '8':R=0x80;
		break;
	case '9':R=0x90;
		break;
	case 'a':
	case 'A':R=0xA0;
		break;
	case 'b':
	case 'B':R=0xB0;
		break;
	case 'c':
	case 'C':R=0xC0;
		break;
	case 'd':
	case 'D':R=0xD0;
		break;
	case 'e':
	case 'E':R|=0x0E;
		break;
	case 'f':
	case 'F':R=0xF0;
		break;
    }
    switch (s){
	case '1':R|=0x01;
		break;
	case '2':R|=0x02;
		break;
	case '3':R|=0x03;
		break;
	case '4':R|=0x04;
		break;
	case '5':R|=0x05;
		break;
	case '6':R|=0x06;
		break;
	case '7':R|=0x07;
		break;
	case '8':R|=0x08;
		break;
	case '9':R|=0x09;
		break;
	case 'a':
	case 'A':R|=0x0A;
		break;
	case 'b':
	case 'B':R|=0x0B;
		break;
	case 'c':
	case 'C':R|=0x0C;
		break;
	case 'd':
	case 'D':R|=0x0D;
		break;
	case 'e':
	case 'E':R|=0x0E;
		break;
	case 'f':
	case 'F':R|=0x0F;
		break;
    }
    return R;
}
