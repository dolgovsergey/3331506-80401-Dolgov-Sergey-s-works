#include "mcu_support_package/inc/stm32f10x.h"


#include <stdbool.h>   
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "math.h"

#define PARAMS_SIZE  8
#define LINES_SIZE  30

int pc8 = 1<<8;
int pc9 = 1<<9;
int pb8 = 1<<8;
int pb9 = 1<<9;
int states = 30000;

typedef  enum  {line,radius,vertical_line} Line_type;

typedef struct {
	Line_type type;
	double params[PARAMS_SIZE];
	
} Line;
Line lines[LINES_SIZE];

double X3, X4, Xr, Yr;
//double main_arr[30][8];
double initial_position_for_next_thing[3];
double buf = 0;
double ip_arr[29];
double current_pos[3];
double y_step_size = 0.012;
double x_step_size = 0.012;
double cutting_depth  = 1;
double x_sp = 0;
double y_sp = -40;
double sector_size = 10;
int current_ip_line = 0;
int ipy_marker = 0;
int ipx_marker = 0;
int types_arr[29], cw = 0;// clock wise (in angle fun count)
int number_of_func = 0;
int additional_delay = 100000;

////prototypes
double char_sym_to_double_sym(char sym);

double char_to_double(char str[], int length);

double paste_number_from_str(char str[], int beg, int len);

int function_num(double x);

double deg_to_rad(double angle);

void angle_function_count(double X0, double Y0, double L1, double a1, double a2, double r);

double current_func_value_count(double x);

int intersection_check(double x, double y);

void fill_main_arr(char str[]);

bool is_number(char sym);

int find_end_pos(char str[], int beg_pos);

void fill_begins_and_ends(int number_of_functions);

int strlenfix(char str[]);

int get_to_pos(double x,double y);

//void forward_y_step();

//void forward_x_step();

//void back_y_step();

//void back_x_step();

void delay (long time_in_ms);

void fill_ip_array(int number_of_functions );

void final_pass(double X_begin, double X_end);

///functions

int find_end_pos(char str[], int beg_pos) {
    int result = 0;
    for (int i = beg_pos; i < beg_pos + 100; i++) {
        if (is_number(str[i]) != 1) {
            result = i;
            return result;
        }
    }
    return result;
}// getting string of numbers and letters and beginning position / returning position of 1st non number if there's no such returns 0 so eng marker is strongly needed


void forward_y_step(){
	
  	GPIOC->ODR |=pc9;	//SET dir
		GPIOC->ODR |=  pc8;
		for (int j = 0;j<states;j++){}
		GPIOC->ODR &= ~pc8;
		for (int j = 0;j<states;j++){}
    GPIOC->ODR &= ~pc9;	//CLEAR dir
}///ONLY STEPS NO DELAYS NO CHECKS

void forward_x_step(){
	
  	GPIOB->ODR |=pb9;	//SET dir
  	GPIOB->ODR |=  pb8;
		for (int j = 0;j<states;j++){}
		GPIOB->ODR &= ~pb8;
		for (int j = 0;j<states;j++){}
    GPIOB->ODR  &= ~pb9;	//Clear dir
}

void back_y_step(){
	
		GPIOC->ODR &= ~pc9;	//SET dir
    GPIOC->ODR |=  pc8;
		for (int j = 0;j<states;j++){}
		GPIOC->ODR &= ~pc8;
		for (int j = 0;j<states;j++){}	


}

void back_x_step(){



		GPIOB->ODR &= ~pb9;	//SET dir
	  GPIOB->ODR |=  pb8;
		for (int j = 0;j<states;j++){}
		GPIOB->ODR &= ~pb8;
		for (int j = 0;j<states;j++){}
}


void delay (long time_in_ms){
    long num_of_ticks = time_in_ms*72000;// time in sec * freq
    for (long i = 0 ; i < num_of_ticks;i++){
    }
}//timer based on frequency of processor / gets time in milliseconds (adjustable) :D / limited with long type size devided be processor frequency


int strlenfix(char str[]) {
    int res = 0;
    for (int i = 0; i < 100; i++) {
        if (is_number(str[i]) == 1) {}
        else {
            res = i;
            break;
        }
    }
    return res;
}// no needed though can be / finds length of string of numbers (basically the same as find end pos)

// returns 0 or 1 (not boolean because I want so) based on symbool checking accodig to list of acceptable ones
bool is_number(char sym) {
    switch (sym) {

          
        case '-':
          return true;
        case '.':
          return true;
        case ',':
          return true;

        default:
					return isdigit(sym);
    }
   
}

double char_sym_to_double_sym(char sym) {
	return sym-'0'; //вычитая код "0" мы получаем порядковый номер для цифры, который приводится к int 
    
}//needed in char to double function / onely numbers , no signs or smth else

double char_to_double(char str[], int length) {
    double result = 0;
    int ep = 0;
    int point_pos = 0;
    for (int i = 0; i < length; i++) {
        if (str[i] == '.' || str[i] == ',') {
            point_pos = i;
            break;
        }
    }
    if (point_pos == 0) { point_pos = length; }
    float mul10 = 1;
    for (int i = point_pos - 1; i >= 0; i--) {
        result += mul10 * char_sym_to_double_sym(str[i]);
        //  //printf("   %c   ",str[i]);
        mul10 *= 10;
        //   //printf("\n%lf",result);
    }
    mul10 = 1;
    if (length - point_pos - 1 < 6) {
        ep = length - point_pos;
    } else {
        ep = 6;
    }
    for (int i = point_pos + 1; i < (point_pos + ep); i++) {
        mul10 /= 10;
        result += mul10 * char_sym_to_double_sym(str[i]);

    }
    if (str[0] == '-') { result *= -1; }
    return result;
}// converts char to double / no protection from symbools exept -, numbers, "," or "."

double paste_number_from_str(char str[], int beg,
                             int len) {// str - from where, beg - which symbol is first, len - how many symbols to copy
    double result = 0;
    char arr[30];
    for (int i = beg; i < beg + len; i++) {
        arr[i - beg] = str[i];
        // //printf("%c", str[i]);
    }
    result = char_to_double(arr, len);
    return result;
}

int function_num(double x) {//returns number of line where func is described
    int result = 0;
    for (int i = 1; i < number_of_func; i++) {
        if ((x <= main_arr[i][0]) && (x >= main_arr[i + 1][0])) {
            result = i;
            if (main_arr[i][4] == 90 || main_arr[i][4] == 270) { result++; }
            return result;
        }
    }

    return number_of_func;//if its last vertical line
}

double deg_to_rad(double angle) {
    double result;
    result = angle * 3.1415926 / 180;
    return result;
}//uses pi as 3.145926 if more accuracy is needed - add additional digits

void angle_function_count(double X0, double Y0, double L1, double a1, double a2, double r) {
    double Y1 = L1 * sin(deg_to_rad(a1)) + Y0; //there r some stuff better described in painting1
 //    //printf("\nY1  %f", Y1);             //basically, it is pure geometry, so use painting1 to understand it easier
    double X1 = L1 * cos(deg_to_rad(a1)) + X0;
   //  //printf("\nX1  %f", X1);
    double b = a1 - (a2 - 180);
  //   //printf("\nb  %f", b);
    double Lr = r / (sin(deg_to_rad(b / 2)));
   //  //printf("\nLr  %f", Lr);
    Xr = Lr * cos(deg_to_rad(a2 + (b / 2))) + X1;
  //   //printf("\nxr  %f", Xr);
    Yr = Lr * sin(deg_to_rad(a2 + (b / 2))) + Y1;
  //   //printf("\nYr  %f", Yr);
    double Lx4 = Lr * sin(deg_to_rad(90 - (b / 2)));
  //   //printf("\nLx4  %f", Lx4);
    X4 = Lx4 * cos(deg_to_rad(a2)) + X1;
  //   //printf("\nX4  %f", X4);
    X3 = Lx4 * cos(deg_to_rad(a2 + b)) + X1;
 //    //printf("\nX3  %f", X3);
    cw = 0;
    if ((a2 - a1) < 0.0) {
        cw = 1;
    //     //printf("\n case 2");
        b = 180 - a1 + a2;
     //    //printf("\nb  %f", b);
        Xr = Lr * cos(deg_to_rad((b / 2) - (180 - a1))) + X1;
       //  //printf("\nXr  %f", Xr);
        Yr = Lr * sin(deg_to_rad((b / 2) - (180 - a1))) + Y1;
     //    //printf("\nYr  %f", Yr);
        X4 = X1 + Lx4 * cos(deg_to_rad(a2 + 180));
     //    //printf("\nX4  %f", X4);
        X3 = X1 + Lx4 * cos(deg_to_rad(a1));
     //    //printf("\nX3  %f", X3);
    }

} //counts connection of 2 straight lines by radius / finds Xr Yr X3 - beginning of radius from right to left X4 - end of radius

double current_func_value_count(double x) {
    double X0 = 0, Y0 = 0, angle = 0, length = 0, radius = 0, k = 0;
    double result = 0;
    //  //printf("\n func num (x) = %d",function_num(x));

    int type = types_arr[function_num(x)];
    //  //printf("\ntypes arr(x)= %d",types_arr[function_num(x)]);
    // //printf("\n check current func value count \n type  is %d\n number of line is %d", type, function_num(x));
    switch (type) {
        case 0:
            break;
        case 1:
            X0 = main_arr[function_num(x)][0];//x begin
            Y0 = main_arr[function_num(x)][1];// y begin
            angle = main_arr[function_num(x)][4];
            k = tan(deg_to_rad(angle));
            result = k * (x - X0) + Y0;//doesnt care about length
            break;
        case 2://func num skips vertical lines
            break;
        case 3:

            angle_function_count(main_arr[function_num(x) - 1][0], main_arr[function_num(x) - 1][1],
                                 main_arr[function_num(x) - 1][3],
                                 main_arr[function_num(x) - 1][4], main_arr[function_num(x) + 1][4],
                                 main_arr[function_num(x)][3]);
            result = Yr - sin((acos((x - Xr) / main_arr[function_num(x)][3]))) * main_arr[function_num(x)][3];


            break;
        case 4:
            angle_function_count(main_arr[function_num(x) - 1][0], main_arr[function_num(x) - 1][1],
                                 main_arr[function_num(x) - 1][3],
                                 main_arr[function_num(x) - 1][4], main_arr[function_num(x) + 1][4],
                                 main_arr[function_num(x)][3]);
            result = Yr + sin((acos((x - Xr) / main_arr[function_num(x)][3]))) * main_arr[function_num(x)][3];
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
    }
    return result;

}

int intersection_check(double x, double y) {
    int result = 0;
    if (current_func_value_count(x) > y) {
        result = 0;
    } else {
        result = 1;
    }
    return result;
}

void fill_main_arr(char str[]) {/// filling main arr, types arr, counting number of functions
    int i = 0;
    int beg_pos = 0, end_pos = 0;
    int end_marker = 0;
    int current_line = 1;
    unsigned int length = strlen(str);
    while (end_marker == 0) {
        //  //printf("\nsykle\n");
        //  //printf("\n num of func %d \n", number_of_func);

        switch (str[i]) {
            case 'l': {
                i++;
                beg_pos = i;
                end_pos = find_end_pos(str, beg_pos);
                main_arr[current_line][3] = paste_number_from_str(str, beg_pos, end_pos - beg_pos); //scanf() /atof() /strtof()
                i = end_pos;
              //  types_arr[current_line] = 1;
							lines[current_line].type = line;
							#error так же для остальных
                if (str[i] == 'a') {
                    i++;
                    beg_pos = i;
                    end_pos = find_end_pos(str, beg_pos);
                    main_arr[current_line][4] = paste_number_from_str(str, beg_pos, end_pos - beg_pos);
                    i = end_pos;
                    current_line++;
                    number_of_func++;
                } else {
                     //printf(" requested angle a at pos %d ", i);
                    end_marker = 1;
                }
            }
                break;
            case 'r': {
                i++;
                beg_pos = i;
                end_pos = find_end_pos(str, beg_pos);
                main_arr[current_line][3] = paste_number_from_str(str, beg_pos, end_pos - beg_pos);
                i = end_pos;
                types_arr[current_line] = 3;
                current_line++;
                number_of_func++;
            }
                break;
        }
        //i++;
        if (i >= length - 1) { end_marker = 1; }
    }

}

void fill_begins_and_ends(int number_of_functions) {//+define vertical lines
    for (int current_line = 1; current_line <= number_of_functions; current_line++) {
        switch (types_arr[current_line]) {//we can start drawing only with line
            case 1: {
                if (current_line == 1) {//if its 1 st line
                    main_arr[1][0] = main_arr[0][0];//x begin
                    main_arr[1][1] = main_arr[0][1];//y begin
                    if (main_arr[1][4] == 90 || main_arr[1][4] == 270) {//if 1 st line is vertical
                        main_arr[1][2] = main_arr[1][0];//x end if line vertical same as..
                        main_arr[2][0] = main_arr[1][0];//next x begin = ^ x end ... dont ask why
                        main_arr[2][1] =
                                main_arr[1][1] + main_arr[1][3] * sin(deg_to_rad(main_arr[1][4]));//next y begin
                        types_arr[1] = 2;//mark line as vertical
                        // if line is vertical
                    } else {
                        main_arr[1][2] = main_arr[1][0] + main_arr[1][3] * cos(deg_to_rad(main_arr[1][4]));//x end
                        main_arr[2][0] = main_arr[1][0] + main_arr[1][3] * cos(deg_to_rad(main_arr[1][4])); //next x beg
                        main_arr[2][1] = main_arr[1][1] + main_arr[1][3] * sin(deg_to_rad(main_arr[1][4]));// next y beg
                    }


                } else {
                    if (main_arr[current_line][4] == 90 || main_arr[current_line][4] == 270) {//if line is vertical
                        main_arr[current_line][2] = main_arr[current_line][0];// x end
                        main_arr[current_line + 1][1] = main_arr[current_line][1] + main_arr[current_line][3] *
                                                                                    sin(deg_to_rad(
                                                                                            main_arr[current_line][4]));// next y beg
                        main_arr[current_line + 1][0] = main_arr[current_line][0] + main_arr[current_line][3] *
                                                                                    cos(deg_to_rad(
                                                                                            main_arr[current_line][4]));// next x beg
                        types_arr[current_line] = 2;//mark vertical line
                    } else {//if line is not vertical
                        main_arr[current_line][2] = main_arr[current_line][0] + main_arr[current_line][3] *
                                                                                cos(deg_to_rad(
                                                                                        main_arr[current_line][4]));// x end
                        main_arr[current_line + 1][0] = main_arr[current_line][0] + main_arr[current_line][3] *
                                                                                    cos(deg_to_rad(
                                                                                            main_arr[current_line][4]));// next x beg
                        main_arr[current_line + 1][1] = main_arr[current_line][1] + main_arr[current_line][3] *
                                                                                    sin(deg_to_rad(
                                                                                            main_arr[current_line][4]));// next y beg
                    }
                }

            }
                break;
            case 3: {
                main_arr[current_line + 1][0] = main_arr[current_line][0];
                main_arr[current_line + 1][1] = main_arr[current_line][1];
                main_arr[current_line][2] = main_arr[current_line - 1][2];


            }
                break;
        }
    }
    for (int current_line = 1; current_line <= number_of_functions; current_line++) {
        switch (types_arr[current_line]) {
            case 3: {
                angle_function_count(main_arr[current_line - 1][0], main_arr[current_line - 1][1],
                                     main_arr[current_line - 1][3], main_arr[current_line - 1][4],
                                     main_arr[current_line + 1][4],
                                     main_arr[current_line][3]);//counting x3 x4 xr based on previous and future line
                if (cw == 1) {


                    buf = tan(deg_to_rad(main_arr[current_line + 1][4])) * (X4 - main_arr[current_line + 1][0]) +
                          main_arr[current_line + 1][1];
                    main_arr[current_line + 1][1] = buf; //count next y begin
                    // result = Yr - sin((acos((x - Xr) / main_arr[function_num(x)][3]))) * main_arr[function_num(x)][3];
                    types_arr[current_line] = 3;
                }//here we ll not add next line y begin as it ll be added by last line
                else {
                    //  result = Yr + sin((acos((x - Xr) / main_arr[function_num(x)][3]))) * main_arr[function_num(x)][3];
                    buf = tan(deg_to_rad(main_arr[current_line + 1][4])) * (X4 - main_arr[current_line + 1][0]) +
                          main_arr[current_line + 1][1];
                    main_arr[current_line + 1][1] = buf; //count next y begin
                    types_arr[current_line] = 4;
                }
                main_arr[current_line][0] = X3;
                main_arr[current_line][2] = Xr;//next y begin is counted earlier
                main_arr[current_line][4] = Yr;
                main_arr[current_line + 1][0] = X4;



            }
                break;

        }
    }
}

void fill_ip_array(int number_of_functions ){
    current_ip_line= 0;
    for (int current_line = 1 ; current_line <= number_of_functions;current_line++ ){
        if (types_arr[current_line]==1&&types_arr[current_line+1]==1){//2 lines
            if (main_arr[current_line+1][4]<=180){
                ip_arr[current_ip_line] = main_arr[current_line+1][0]-0.00001;
                current_ip_line++;
            }
        }

        if (types_arr[current_line]==2&&types_arr[current_line+1]==1){//if 1st of two lines is vertical
            if (main_arr[current_line+1][4]<=180){
                ip_arr[current_ip_line] = main_arr[current_line+1][0]-0.00001;
                current_ip_line++;
            }
        }
        if (types_arr[current_line]==3&&main_arr[current_line+1][4]<=180){//radius
            ip_arr[current_ip_line] = main_arr[current_line][2]-0.00001;
            current_ip_line++;
        }
    }

}

int get_to_pos(double x,double y){//getting to position checking steps

    while (current_pos[2]-y_step_size-0.000001 > y){//top to bottom
        if(intersection_check(current_pos[1], current_pos[2]-y_step_size)==0){
            current_pos[2]-=y_step_size;
            back_y_step();
            delay(additional_delay);
             //printf("\n c.p. Y = %f", current_pos[2]);
        } else{
            ipy_marker = 1;
            return 0;
        }
    }
    while ( current_pos[2]+y_step_size+0.000001 < y){//bottom to top
        if(intersection_check(current_pos[1], current_pos[2]+y_step_size)==0){
            current_pos[2]+=y_step_size;
            forward_y_step();
            delay(additional_delay);
             //printf("\n c.p. Y = %f", current_pos[2]);

        } else{
            ipy_marker = 1;
            return 0;
        }
    }
    while ( current_pos[1]+x_step_size+0.000001 < x){//left to right
        if(intersection_check(current_pos[1]+x_step_size, current_pos[2])==0){
            current_pos[1]+=x_step_size;
            forward_x_step();
            delay(additional_delay);
             //printf("\n c.p. X = %f", current_pos[1]);

        } else{
            ipx_marker = 1;
            return 0;
        }
    }
    while ( current_pos[1]-x_step_size-0.000001 > x){//right to left
        if(intersection_check(current_pos[1]-x_step_size, current_pos[2])==0){
            current_pos[1]-=x_step_size;
            back_x_step();
            delay(additional_delay);
             //printf("\n c.p. X = %f", current_pos[1]);

        } else{
            ipx_marker = 1;
            return 0;
        }
    }
    return 0;
}//looks ok

void go_to_sertain_pos (double x,double y){
    while (current_pos[2]-y_step_size-0.000001 > y){//top to bottom

            current_pos[2]-=y_step_size;
            back_y_step();
            //delay(additional_delay);
             //printf("\n c.p. Y = %f", current_pos[2]);

    }
    while ( current_pos[2]+y_step_size+0.000001 < y){//bottom to top

            current_pos[2]+=y_step_size;
            forward_y_step();
            //delay(additional_delay);
             //printf("\n c.p. Y = %f", current_pos[2]);


    }
    while ( current_pos[1]+x_step_size+0.000001 < x){//left to right

            current_pos[1]+=x_step_size;
            forward_x_step();
            //delay(additional_delay);
             //printf("\n c.p. X = %f", current_pos[1]);


    }
    while ( current_pos[1]-x_step_size-0.000001 > x){//right to left

            current_pos[1]-=x_step_size;
            back_x_step();
           // delay(additional_delay);
             //printf("\n c.p. X = %f", current_pos[1]);


    }
}

int sector_left_cutting (double X_begin, double X_end) {
    get_to_pos(current_pos[1],main_arr[0][2]-5);
    get_to_pos(X_begin,current_pos[2]);
    int stack_marker = 0;
    int end_marker = 0;
    int last_ip_marker = current_ip_line;
     //printf("\n cur ip line %d",current_ip_line);

    current_ip_line = 0;
 x_sp = X_begin;
 y_sp  = main_arr[0][2];//y offset
double right_sector_edge = X_begin;
double left_sector_edge  = X_begin-sector_size;
if(left_sector_edge< X_end){
    left_sector_edge = X_end;
}

next_ip:
next_cut:
if (stack_marker == 1){
y_sp  = current_pos[2]-y_step_size-0.00001;
}
if (stack_marker == 2){
    x_sp = ip_arr[current_ip_line];
    y_sp = current_func_value_count(x_sp)-0.01;
    current_ip_line++;
    if (current_ip_line > last_ip_marker){
        stack_marker = 3;
        goto next_sector;
    }
}
if (stack_marker ==3){
    x_sp = right_sector_edge;
    y_sp = main_arr[0][2];
}
    if (stack_marker ==4){
         //printf("\n  stack marker = 4 exiting");
        return 0;
    }
     //printf("\n 614 line check x_sp = %f y_sp = %f ",x_sp,y_sp);
     //printf("\nleft edge %f  right edge %f",left_sector_edge, right_sector_edge);

    get_to_pos(x_sp,y_sp);
ipx_marker = 0;
ipy_marker = 0;
while (current_pos[2]-y_sp<cutting_depth){//cutting in
    get_to_pos(current_pos[1],current_pos[2]+ y_step_size+0.00001);
    if(ipy_marker==1|| current_pos[2]+y_step_size>=0){
        ipy_marker = 0;
        get_to_pos(current_pos[1]-x_step_size-0.00001,current_pos[2]);///idk
        if(ipx_marker==1|| current_pos[1]-x_step_size<left_sector_edge){
            ipx_marker=0;
            if(ip_arr[current_ip_line]>left_sector_edge){
                stack_marker = 2;
                 //printf("\n goto next ip");
                goto next_ip;
            }
            stack_marker = 3;
             //printf("\n goto next sector");
            goto next_sector;
            }//so it should stop here
    }
}
x_sp = current_pos[1]+x_step_size;
while (current_pos[1]-x_step_size-0.00001>left_sector_edge){
    get_to_pos(current_pos[1]-x_step_size-0.00001,current_pos[2]);
    if(ipx_marker==1||current_pos[1]-x_step_size-0.00002<left_sector_edge){//if stop no need to deside case
        ipx_marker = 0;
        stack_marker = 1;
         //printf("\n goto next cut");
        goto next_cut;
    }
}
stack_marker = 1;
     //printf("\n goto next cut");
goto next_cut;
    next_sector:
    right_sector_edge = left_sector_edge;
    left_sector_edge  -=sector_size;
    if(left_sector_edge< X_end){
         //printf("\n655");
        left_sector_edge = X_end;
    }
    if (right_sector_edge - left_sector_edge>0){
        if(right_sector_edge - left_sector_edge < 0.001+x_step_size){
        stack_marker = 4;
         //printf("\n713");
        }
    } else if (left_sector_edge- right_sector_edge  < 0.001+x_step_size){
         //printf("\n%f",left_sector_edge- right_sector_edge);
         //printf("\n%f",left_sector_edge);

        stack_marker = 4;
             //printf("\n720");

        }

     //printf("\n goto next cut");
    goto next_cut;

    }

void final_pass(double X_begin, double X_end){
    get_to_pos(current_pos[1],main_arr[0][2]-5);
    get_to_pos(X_begin,current_pos[2]);
    get_to_pos(current_pos[1],current_func_value_count(current_pos[1]));
     //printf("\n Final pass");
    while (current_pos[1]-x_step_size-0.00001>X_end){
       //  //printf("\n current x %f current y %f next x pos %f next y pos %f",current_pos[1],current_pos[2],current_pos[1]-x_step_size-0.00001,current_func_value_count(current_pos[1]-x_step_size-0.00001));
        go_to_sertain_pos(current_pos[1]-x_step_size-0.00001,current_func_value_count(current_pos[1]-x_step_size-0.00001));
      //   //printf("\n current x %f current y %f next x pos %f next y pos %f",current_pos[1],current_pos[2],current_pos[1]-x_step_size-0.00001,current_func_value_count(current_pos[1]-x_step_size-0.00001));

       //  //printf("\nloop");
        //delay(100);
    }
}

int main() {
	
	//0 all
	RCC->APB2ENR =0;


	//microcontroller  settings
	RCC->APB2ENR |= 1<<4; //clock c port
	RCC->APB2ENR |= 1<<3; //clock b port
	GPIOC->CRH   =0;
	GPIOB->CRH   =0;
	GPIOC->CRH   |= 0x30; //11 00 00 //10mhz pc9 output push pull
	GPIOC->CRH   |= 1;  //11	//10mhz pc8 output push pull
	GPIOB->CRH   |= 0x30;	//11 00 00 same
	GPIOB->CRH   |= 1;	//11 same

	//program settings
	main_arr[0][0] = -12;//X0 offset from 0 point
	main_arr[0][1] = -17;// Y0 offstt from 0 point
	main_arr[0][2] = -20;// offset Y
	current_pos[1] = -14; //current X position IMPORTANT TO BE BETWEEN BEGINOF CUTTING AND END
	current_pos[2] = -25; // current Y position NEED TO BE LESS THEN OFFSET I MEAN MORE BY MODULE BUT LESS IN GENEREL
	initial_position_for_next_thing[1] = current_pos[1];
	initial_position_for_next_thing[2] = current_pos[2];
	//ALL measurements in mm


	///                0123456789 123456789 12345678
	fill_main_arr("l5 a180 r2.5 l3.5 a90 l10 a190");//figure shape 
	fill_begins_and_ends(number_of_func);//dont change
	fill_ip_array(number_of_func);//dont change



	//start cutting 

	sector_left_cutting(-12.1,-24);// from where to where by X 
	final_pass(-12.1,-24);         // from where to where by X 
	go_to_sertain_pos(initial_position_for_next_thing[1], initial_position_for_next_thing[2]);//also used for positioning

	return 0;
}
//USE SPL
