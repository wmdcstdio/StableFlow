﻿#include "shared.h"
#include "solver.h"
Solver S;

void Init(void) {
	S.Init();
	Dye dwhite = Dye(1, 1, 1);
	dwhite.Add_Src(0.4, 0.6, 0.4, 0.6, 1);
	S.colors.push_back(dwhite);
	S.Add_CV(0.4, 0.6, 0.2, 0.3, 10);
	S.Add_CV(0.4, 0.6, 0.7, 0.8, -10);
}


void Process_Click(int button, int state, int y, int x) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
	}
}

void Plot(void) {
	glClear(GL_COLOR_BUFFER_BIT);
	S.Draw();
	glFlush();
	glutSwapBuffers();
}



void Step_Time(int v) {
	S.Step();
	Plot();
	glutTimerFunc(1000 / SHOW_FPS, Step_Time, 1);
}

int main(int argc, char *argv[]) {
	Init();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);//double buffer
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(SHOWW, SHOWH*TOTAL_SCREEN);
	int glut_window = glutCreateWindow("Rasterization");
	glutMouseFunc(&Process_Click);
	glutDisplayFunc(&Plot);
	glutTimerFunc(1000 / SHOW_FPS, Step_Time, 1);
	glutMainLoop();
	return 0;
}