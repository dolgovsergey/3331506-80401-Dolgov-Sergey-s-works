#include <stdio.h>
#include <string.h>
#include <math.h>

double X3, X4, Xr, Yr;//
double zero = 0;// idk if it is correct but it is made to avoid casting types
double main_arr[30][8];//thats the main array ... where all parameters of all functions will be
int number_of_func;

double char_sym_to_double_sym(char sym) {//no need to comment - too simple
    double res = 0;
    switch (sym) {
        case '0' :
            res = 0;
            break;
        case '1' :
            res = 1;
            break;
        case '2' :
            res = 2;
            break;
        case '3' :
            res = 3;
            break;
        case '4' :
            res = 4;
            break;
        case '5' :
            res = 5;
            break;
        case '6' :
            res = 6;
            break;
        case '7' :
            res = 7;
            break;
        case '8' :
            res = 8;
            break;
        case '9' :
            res = 9;
            break;
    }
    return res;
}//needed in char to double function maybe I ll join them
double char_to_double(char str[], int length) {
    double result = 0;
    int ep = 0;
    int point_pos = 0;
    for (int i = 0; i < length; i++) {// find point position
        if (str[i] == '.' || str[i] == ',') {
            point_pos = i;
            break;
        }
    }
    float mul10 = 1;
    for (int i = point_pos - 1; i >= 0; i--) { //convert into double part before point
        result += mul10 * char_sym_to_double_sym(str[i]);
        // printf("   %c   ",str[i]);
        mul10 *= 10;
        //  printf("\n%lf",result);
    }
    mul10 = 1;
    if (length - point_pos - 1 < 6) {// check how many symbols are there after point
        ep = length - point_pos;// if <6 (just a number) then convert all digits
    } else {
        ep = 6;// if not then only 6-1=5 digits... i suppose
    }
    for (int i = point_pos + 1; i < (point_pos + ep); i++) {//converting
        mul10 /= 10;
        result += mul10 * char_sym_to_double_sym(str[i]);

    }
    if (str[0] == '-') {
        result *= -1;
    }
    return result;
}

double deg_to_rad(double angle) {
    double result;
    result = angle * 3.1415926 / 180;
    return result;
}

void angle_function_count(double X0, double Y0, double L1, double a1, double a2, double r) {
    double Y1 = L1 * sin(deg_to_rad(a1)) + Y0; //there r some stuff better described in painting1
    printf("\nY1  %f", Y1);             //basically, it is pure geometry, so use painting1 to understand it easier
    double X1 = L1 * cos(deg_to_rad(a1)) + X0;
    printf("\nX1  %f", X1);
    double b = a1 - (a2 - 180);
    printf("\nb  %f", b);
    double Lr = r / (sin(deg_to_rad(b / 2)));
    printf("\nLr  %f", Lr);
    Xr = Lr * cos(deg_to_rad(a2 + (b / 2))) + X1;
    printf("\nxr  %f", Xr);
    Yr = Lr * sin(deg_to_rad(a2 + (b / 2))) + Y1;
    printf("\nYr  %f", Yr);
    double Lx4 = Lr * sin(deg_to_rad(90 - (b / 2)));
    printf("\nLx4  %f", Lx4);
    X4 = Lx4 * cos(deg_to_rad(a2)) + X1;
    printf("\nX4  %f", X4);
    X3 = Lx4 * cos(deg_to_rad(a2 + b)) + X1;
    printf("\nX3  %f", X3);
    if ((a2 - a1) < zero) {
        printf("\n case 2");
        b = 180 - a1 + a2;
        printf("\nb  %f", b);
        Xr = Lr * cos(deg_to_rad((b / 2) - (180 - a1))) + X1;
        printf("\nXr  %f", Xr);
        Yr = Lr * sin(deg_to_rad((b / 2) - (180 - a1))) + Y1;
        printf("\nYr  %f", Yr);
        X4 = X1 + Lx4 * cos(deg_to_rad(a2 + 180));
        printf("\nX4  %f", X4);
        X3 = X1 + Lx4 * cos(deg_to_rad(a1));
        printf("\nX3  %f", X3);
    }

}

double paste_number_from_str(char str[], int beg, int len) {
    double result = 0;
    char arr[30];
    for (int i = beg; i < beg + len; i++) {
        arr[i - beg] = str[i];
        // printf("%c", str[i]);
    }
    result = char_to_double(arr, len);
    return result;
}

int function_num(double x) {//defines wich is current function number (1 column (or row, idc ) in main_arr)
    int result = 0;
    for (int i = 1; i < number_of_func; i++) {
        if ((x <= main_arr[i][1]) && (x > main_arr[i + 1][1])) {//array includes begins and ends of functions
            result = i;// and some other parameters
            break;
        }
    }
    return result;
}

int main() {
    // examples
    printf("\n%f", char_to_double("13245.2658984", strlen("13245.2658984")));
    printf("\n%f", paste_number_from_str("fgh123456.9845268", 3, 11));

    angle_function_count(0, 0, 34, 41.629, 23.786, 50);
    printf("\nXr %f", Xr);
    printf("\nYr %f", Yr);
    printf("\nX3 %f", X3);
    printf("\nX4 %f", X4);
//few more words: honestly, I m so sorry, but I will have to use goto in one of other functions
//also, its a prototype  (of prototype :D), so some functions can be joined or included
//into each other or main to improve performance
// and I completely dont know why is it so: Clang-Tidy: Potential uncovered code path; add a default label
//also wont you mind if I ll write my code in this style (i found it pretty handy)
    return 0;
}
