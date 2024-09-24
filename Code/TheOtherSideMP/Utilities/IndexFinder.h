/*************************************************************************
AlienKeeper Source File.
Copyright (C), AlienKeeper, 2024.
**************************************************************************/

#pragma once
//#include "sstream"
//In the header .h

class IndexFinder
{
	typedef int (IndexFinder::* method_pointer)();
public:
	template<typename _MethodPtr>
	static size_t getIndexOf(_MethodPtr ptr)
	{
		/*
		reinterpret_cast<IndexFinder*>(&fake_vtable_ptr) - преобразует fake_vtable_ptr в указатель на IndexFinder.
		fake_vtable_ptr должен быть указателем на искусственно созданную структуру, которая имитирует виртуальную таблицу класса IndexFinder.
		*((IndexFinder::method_pointer*)(&ptr)) - получает значение указателя на метод ptr и преобразует его в указатель на функцию-член IndexFinder::method_pointer.
		**(...)(): разыменовывает полученный указатель на функцию-член, вызывая ее.
		return (...)(); - возвращает значение, возвращенное вызываемым методом.
		*/

		IndexFinder* indexFinderPtr = reinterpret_cast<IndexFinder*>(&fake_vtable_ptr);
		IndexFinder::method_pointer* method_pointer = (IndexFinder::method_pointer*)(&ptr);
		IndexFinder::method_pointer method = *(method_pointer);

		return (indexFinderPtr->*method)();
	}

	typedef method_pointer fake_vtable_t[201];
	static fake_vtable_t   fake_vtable;
	static void* fake_vtable_ptr;

protected:
	int method0()
	{
		return 0;
	}
	int method1()
	{
		return 1;
	}
	int method2()
	{
		return 2;
	}
	int method3()
	{
		return 3;
	}
	int method4()
	{
		return 4;
	}
	int method5()
	{
		return 5;
	}
	int method6()
	{
		return 6;
	}
	int method7()
	{
		return 7;
	}
	int method8()
	{
		return 8;
	}
	int method9()
	{
		return 9;
	}
	int method10()
	{
		return 10;
	}
	int method11()
	{
		return 11;
	}
	int method12()
	{
		return 12;
	}
	int method13()
	{
		return 13;
	}
	int method14()
	{
		return 14;
	}
	int method15()
	{
		return 15;
	}
	int method16()
	{
		return 16;
	}
	int method17()
	{
		return 17;
	}
	int method18()
	{
		return 18;
	}
	int method19()
	{
		return 19;
	}
	int method20()
	{
		return 20;
	}
	int method21()
	{
		return 21;
	}
	int method22()
	{
		return 22;
	}
	int method23()
	{
		return 23;
	}
	int method24()
	{
		return 24;
	}
	int method25()
	{
		return 25;
	}
	int method26()
	{
		return 26;
	}
	int method27()
	{
		return 27;
	}
	int method28()
	{
		return 28;
	}
	int method29()
	{
		return 29;
	}
	int method30()
	{
		return 30;
	}
	int method31()
	{
		return 31;
	}
	int method32()
	{
		return 32;
	}
	int method33()
	{
		return 33;
	}
	int method34()
	{
		return 34;
	}
	int method35()
	{
		return 35;
	}
	int method36()
	{
		return 36;
	}
	int method37()
	{
		return 37;
	}
	int method38()
	{
		return 38;
	}
	int method39()
	{
		return 39;
	}
	int method40()
	{
		return 40;
	}
	int method41()
	{
		return 41;
	}
	int method42()
	{
		return 42;
	}
	int method43()
	{
		return 43;
	}
	int method44()
	{
		return 44;
	}
	int method45()
	{
		return 45;
	}
	int method46()
	{
		return 46;
	}
	int method47()
	{
		return 47;
	}
	int method48()
	{
		return 48;
	}
	int method49()
	{
		return 49;
	}
	int method50()
	{
		return 50;
	}
	int method51()
	{
		return 51;
	}
	int method52()
	{
		return 52;
	}
	int method53()
	{
		return 53;
	}
	int method54()
	{
		return 54;
	}
	int method55()
	{
		return 55;
	}
	int method56()
	{
		return 56;
	}
	int method57()
	{
		return 57;
	}
	int method58()
	{
		return 58;
	}
	int method59()
	{
		return 59;
	}
	int method60()
	{
		return 60;
	}
	int method61()
	{
		return 61;
	}
	int method62()
	{
		return 62;
	}
	int method63()
	{
		return 63;
	}
	int method64()
	{
		return 64;
	}
	int method65()
	{
		return 65;
	}
	int method66()
	{
		return 66;
	}
	int method67()
	{
		return 67;
	}
	int method68()
	{
		return 68;
	}
	int method69()
	{
		return 69;
	}
	int method70()
	{
		return 70;
	}
	int method71()
	{
		return 71;
	}
	int method72()
	{
		return 72;
	}
	int method73()
	{
		return 73;
	}
	int method74()
	{
		return 74;
	}
	int method75()
	{
		return 75;
	}
	int method76()
	{
		return 76;
	}
	int method77()
	{
		return 77;
	}
	int method78()
	{
		return 78;
	}
	int method79()
	{
		return 79;
	}
	int method80()
	{
		return 80;
	}
	int method81()
	{
		return 81;
	}
	int method82()
	{
		return 82;
	}
	int method83()
	{
		return 83;
	}
	int method84()
	{
		return 84;
	}
	int method85()
	{
		return 85;
	}
	int method86()
	{
		return 86;
	}
	int method87()
	{
		return 87;
	}
	int method88()
	{
		return 88;
	}
	int method89()
	{
		return 89;
	}
	int method90()
	{
		return 90;
	}
	int method91()
	{
		return 91;
	}
	int method92()
	{
		return 92;
	}
	int method93()
	{
		return 93;
	}
	int method94()
	{
		return 94;
	}
	int method95()
	{
		return 95;
	}
	int method96()
	{
		return 96;
	}
	int method97()
	{
		return 97;
	}
	int method98()
	{
		return 98;
	}
	int method99()
	{
		return 99;
	}
	int method100()
	{
		return 100;
	}
	int method101()
	{
		return 101;
	}
	int method102()
	{
		return 102;
	}
	int method103()
	{
		return 103;
	}
	int method104()
	{
		return 104;
	}
	int method105()
	{
		return 105;
	}
	int method106()
	{
		return 106;
	}
	int method107()
	{
		return 107;
	}
	int method108()
	{
		return 108;
	}
	int method109()
	{
		return 109;
	}
	int method110()
	{
		return 110;
	}
	int method111()
	{
		return 111;
	}
	int method112()
	{
		return 112;
	}
	int method113()
	{
		return 113;
	}
	int method114()
	{
		return 114;
	}
	int method115()
	{
		return 115;
	}
	int method116()
	{
		return 116;
	}
	int method117()
	{
		return 117;
	}
	int method118()
	{
		return 118;
	}
	int method119()
	{
		return 119;
	}
	int method120()
	{
		return 120;
	}
	int method121()
	{
		return 121;
	}
	int method122()
	{
		return 122;
	}
	int method123()
	{
		return 123;
	}
	int method124()
	{
		return 124;
	}
	int method125()
	{
		return 125;
	}
	int method126()
	{
		return 126;
	}
	int method127()
	{
		return 127;
	}
	int method128()
	{
		return 128;
	}
	int method129()
	{
		return 129;
	}
	int method130()
	{
		return 130;
	}
	int method131()
	{
		return 131;
	}
	int method132()
	{
		return 132;
	}
	int method133()
	{
		return 133;
	}
	int method134()
	{
		return 134;
	}
	int method135()
	{
		return 135;
	}
	int method136()
	{
		return 136;
	}
	int method137()
	{
		return 137;
	}
	int method138()
	{
		return 138;
	}
	int method139()
	{
		return 139;
	}
	int method140()
	{
		return 140;
	}
	int method141()
	{
		return 141;
	}
	int method142()
	{
		return 142;
	}
	int method143()
	{
		return 143;
	}
	int method144()
	{
		return 144;
	}
	int method145()
	{
		return 145;
	}
	int method146()
	{
		return 146;
	}
	int method147()
	{
		return 147;
	}
	int method148()
	{
		return 148;
	}
	int method149()
	{
		return 149;
	}
	int method150()
	{
		return 150;
	}
	int method151()
	{
		return 151;
	}
	int method152()
	{
		return 152;
	}
	int method153()
	{
		return 153;
	}
	int method154()
	{
		return 154;
	}
	int method155()
	{
		return 155;
	}
	int method156()
	{
		return 156;
	}
	int method157()
	{
		return 157;
	}
	int method158()
	{
		return 158;
	}
	int method159()
	{
		return 159;
	}
	int method160()
	{
		return 160;
	}
	int method161()
	{
		return 161;
	}
	int method162()
	{
		return 162;
	}
	int method163()
	{
		return 163;
	}
	int method164()
	{
		return 164;
	}
	int method165()
	{
		return 165;
	}
	int method166()
	{
		return 166;
	}
	int method167()
	{
		return 167;
	}
	int method168()
	{
		return 168;
	}
	int method169()
	{
		return 169;
	}
	int method170()
	{
		return 170;
	}
	int method171()
	{
		return 171;
	}
	int method172()
	{
		return 172;
	}
	int method173()
	{
		return 173;
	}
	int method174()
	{
		return 174;
	}
	int method175()
	{
		return 175;
	}
	int method176()
	{
		return 176;
	}
	int method177()
	{
		return 177;
	}
	int method178()
	{
		return 178;
	}
	int method179()
	{
		return 179;
	}
	int method180()
	{
		return 180;
	}
	int method181()
	{
		return 181;
	}
	int method182()
	{
		return 182;
	}
	int method183()
	{
		return 183;
	}
	int method184()
	{
		return 184;
	}
	int method185()
	{
		return 185;
	}
	int method186()
	{
		return 186;
	}
	int method187()
	{
		return 187;
	}
	int method188()
	{
		return 188;
	}
	int method189()
	{
		return 189;
	}
	int method190()
	{
		return 190;
	}
	int method191()
	{
		return 191;
	}
	int method192()
	{
		return 192;
	}
	int method193()
	{
		return 193;
	}
	int method194()
	{
		return 194;
	}
	int method195()
	{
		return 195;
	}
	int method196()
	{
		return 196;
	}
	int method197()
	{
		return 197;
	}
	int method198()
	{
		return 198;
	}
	int method199()
	{
		return 199;
	}
	int method200()
	{
		return 200;
	}
};