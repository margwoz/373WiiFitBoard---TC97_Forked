/*
 * MatrixDriver.h
 *
 *  Created on: Mar 23, 2026
 *      Author: andleo
 */

#ifndef INC_MAX7219DRIVER_H_
#define INC_MAX7219DRIVER_H_

void Matrix_Clear(void);
void Matrix_Init(void);
void Write_Char(int matrix_num, int char_num);
void Write_Pixel(int matrix_num, int row_num, int col);
void Six_Seven(int iterations);
void Matrix_Clear_Cell(int matrix_num);

#endif /* INC_MAX7219DRIVER_H_ */
