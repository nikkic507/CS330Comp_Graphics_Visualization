#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;

void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	int health; //brick health
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		onoff = ON;
		health = 10;
	};

	void drawBrick()
	{
		if (onoff == ON)
		{
			double halfside = width / 2;

			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);

			glVertex2d(x + halfside, y + halfside);
			glVertex2d(x + halfside, y - halfside);
			glVertex2d(x - halfside, y - halfside);
			glVertex2d(x - halfside, y + halfside);

			glEnd();
		}
	}
};


class Circle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.01;
	int health;
	int direction; // 1=up 2=right 3=down 4=left 5 = up right   6 = up left  7 = down right  8= down left
	ONOFF onoff;
	

	Circle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
		onoff = ON;
		health = 2;
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}
		else if (brk->brick_type == DESTRUCTABLE && brk->onoff == ON && brk->health>0 && onoff == ON)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->health--;
				if (brk->health <= 0) //check brick health
				{
					brk->onoff = OFF;
				}
				direction = GetRandomDirection();
				x = x + 0.03;
				y = y + 0.04;
			}
		}
	}

	int GetRandomDirection()
	{
		return (rand() % 8) + 1;
	}

	void MoveOneStep()
	{
		
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			if (y > -1 + radius)
			{
				y -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			if (x < 1 - radius)
			{
				x += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				direction = GetRandomDirection();
			}
		}
	}

	void DrawCircle()
	{
		
			glColor3f(red, green, blue);
			glBegin(GL_POLYGON);
			for (int i = 0; i < 360; i++) {
				float degInRad = i * DEG2RAD;
				glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);
			}
			glEnd();
		}
};


vector<Circle> world;


int main(void) {
	srand(time(NULL));

	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(480, 480, "8-2 Assignment", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	Brick brick(DESTRUCTABLE, 0.5, -0.33, 0.20, 1, 1, 0);
	Brick brick2(DESTRUCTABLE, -0.5, 0.20, 0.20, 0, 1, 0);
	Brick brick3(DESTRUCTABLE, -0.5, -0.33, 0.20, 0, 1, 1);
	Brick brick4(REFLECTIVE, 0.7, 0.6, 0.20, 1, 0.5, 0.5);//coral
	Brick brick5(DESTRUCTABLE, -0.9, 0.80, 0.20, 1, 1, 1);//white
	Brick brick6(DESTRUCTABLE, 0.9, 0.80, 0.20, 0, 1, 1);//blue
	Brick brick7(DESTRUCTABLE, -0.7, 0.80, 0.20, 0, 1, 0);//green
	Brick brick8(DESTRUCTABLE, 0.7, 0.80, 0.20, 1, 1, 1);//white
	Brick brick9(DESTRUCTABLE, 0.5, 0.20, 0.20, 1, 0, 0);//red
	Brick brick10(REFLECTIVE, -0.7, 0.60, 0.20, 1, 0.5, 0.5);//coral
	Brick brick11(REFLECTIVE, -0.5, 0.40, 0.20, 1, 0.5, 0.5);//coral
	Brick brick12(REFLECTIVE, 0.5, 0.40, 0.20, 1, 0.5, 0.5);//coral
	Brick brick13(REFLECTIVE, -0.3, 0.20, 0.20, 1, 0.5, 0.5);//coral
	Brick brick14(REFLECTIVE, 0.3, 0.20, 0.20, 1, 0.5, 0.5);//coral
	Brick brick15(DESTRUCTABLE, 0.0, 0.20, 0.20, 0, 1, 1); //blue
	Brick brick16(DESTRUCTABLE, -0.9, 0.00, 0.20, 1, 1, 1);//white
	Brick brick17(DESTRUCTABLE, -0.7, 0.00, 0.20, 0, 1, 0); //green
	Brick brick18(DESTRUCTABLE, -0.5, 0.00, 0.20, 0, 1, 1); //blue
	Brick brick19(DESTRUCTABLE, -0.3, 0.00, 0.20, 1, 1, 1); //white
	Brick brick20(DESTRUCTABLE, -0.1, 0.00, 0.20, 0, 1, 0); //green
	Brick brick21(DESTRUCTABLE, 0.1, 0.00, 0.20, 0, 1, 1);//blue
	Brick brick22(DESTRUCTABLE, 0.3, 0.00, 0.20, 1, 1, 1); //white
	Brick brick23(DESTRUCTABLE, 0.5, 0.00, 0.20, 0, 1, 0); //green
	Brick brick24(DESTRUCTABLE, 0.7, 0.00, 0.20, 0, 1, 1); //blue
	Brick brick25(DESTRUCTABLE, 0.9, 0.00, 0.20, 1, 1, 1); //white
	Brick brick26(REFLECTIVE, 0, -0.8, 0.25, 1, 0.5, 0.5);//circle launcher
	while (!glfwWindowShouldClose(window)) {
		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			world[i].CheckCollision(&brick);
			world[i].CheckCollision(&brick2);
			world[i].CheckCollision(&brick3);
			world[i].CheckCollision(&brick4);
			world[i].CheckCollision(&brick5);
			world[i].CheckCollision(&brick6);
			world[i].CheckCollision(&brick7);
			world[i].CheckCollision(&brick8);
			world[i].CheckCollision(&brick9);
			world[i].CheckCollision(&brick10);
			world[i].CheckCollision(&brick11);
			world[i].CheckCollision(&brick12);
			world[i].CheckCollision(&brick13);
			world[i].CheckCollision(&brick14);
			world[i].CheckCollision(&brick15);
			world[i].CheckCollision(&brick16);
			world[i].CheckCollision(&brick17);
			world[i].CheckCollision(&brick18);
			world[i].CheckCollision(&brick19);
			world[i].CheckCollision(&brick20);
			world[i].CheckCollision(&brick21);
			world[i].CheckCollision(&brick22);
			world[i].CheckCollision(&brick23);
			world[i].CheckCollision(&brick24);
			world[i].CheckCollision(&brick25);
			world[i].CheckCollision(&brick26);
			world[i].MoveOneStep();
			world[i].DrawCircle();

		}

		brick.drawBrick();
		brick2.drawBrick();
		brick3.drawBrick();
		brick4.drawBrick();
		brick5.drawBrick();
		brick6.drawBrick();
		brick7.drawBrick();
		brick8.drawBrick();
		brick9.drawBrick();
		brick10.drawBrick();
		brick11.drawBrick();
		brick12.drawBrick();
		brick13.drawBrick();
		brick14.drawBrick();
		brick15.drawBrick();
		brick16.drawBrick();
		brick17.drawBrick();
		brick18.drawBrick();
		brick19.drawBrick();
		brick20.drawBrick();
		brick21.drawBrick();
		brick22.drawBrick();
		brick23.drawBrick();
		brick24.drawBrick();
		brick25.drawBrick();
		brick26.drawBrick();
		
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}


void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		double r, g, b;
		r = rand() / 10000;
		g = rand() / 10000;
		b = rand() / 10000;
		Circle B(0, -0.8, 02, 3, 0.03, r, g, b);
		world.push_back(B);
	}
}