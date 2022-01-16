#include <vector>
#include <iostream>
#include <algorithm>
#include <string>
#include <math.h>
#include <sstream>
#include <iomanip>

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"

#include "CircleRenderer.h"
#include "LineRenderer.h"


int Factorial(int x) {
	if (x > 1)
		return x * Factorial(x - 1);
	else
		return 1;
}

int BinomialCoefficiant(int n, int k)
{
	return Factorial(n) / (Factorial(k) * Factorial(n - k));
}

class BezierCurve {
public:
	void RegisterPoint(float x, float y) {
		points.push_back({x,y});
	}

	void ClearPoints() {
		points.clear();
	}

	std::vector<glm::vec2> GetCurve() {
		std::vector<glm::vec2> curvePoints;

		float curveStart = points[0].x;

		float curveLength = 0.0f;

		for (int i = 0; i < points.size() - 1; i++)
		{
			curveLength += abs(sqrt(pow(points[i + 1].x - points[i].x, 2) + pow(points[i + 1].y - points[i].y, 2)));
		}

		float accuracy = 1.0f;

		float currentPoint = 0.0f;
		float currentCurveLength = 0.0f;

		while (currentPoint < curveLength)
		{
			float u = currentPoint / curveLength;

			int k = points.size() - 1;
			glm::vec2 point(0.0f,0.0f);

			for (int i = 0; i <= k; i++)
			{
				point += (float)(BinomialCoefficiant(k, i) * pow(1 - u, k - i) * pow(u, i))* points[i];

			}

			curvePoints.push_back(point);
			currentPoint += accuracy;
		}

		return curvePoints;
	}

public:
	std::vector<glm::vec2> points; // kontrolne tocke
};

int main ()
{
	GLFWwindow* window;

	if (!glfwInit())
		std::cout << "Error : could not initilize GLFW";
	int width = 1000;
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(width, width * 9/16, "Hello World", NULL, NULL);
	if (!window)
	{
		std::cout << "Error : could not create window";
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		std::cout << "Error : could not initilize Glad";

	glfwSwapInterval(1);

	bool mouseHeld = false;

	InitCircleRendering(100);
	InitLineRendering();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	BezierCurve curve;
	curve.RegisterPoint(500.0, 100.0);
	curve.RegisterPoint(600.0, 300.0);
	curve.RegisterPoint(750.0, 110.0);

	BezierCurve curve2;
	BezierCurve curve3;
	BezierCurve curve4;

	curve2.RegisterPoint(510.0, 50.0);
	curve2.RegisterPoint(620.0, 80.0);
	curve2.RegisterPoint(800.0, 50.0);


	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glClear(GL_COLOR_BUFFER_BIT);

		for (glm::vec2& vec : curve.points) {
            RenderCircle(glm::vec2(vec.x, vec.y),5);
		}

		for (glm::vec2& vec : curve2.points) {
            RenderCircle(glm::vec2(vec.x, vec.y), 5);
		}

/*
		for (int i = 0; i < curve.points.size(); i++)
		{
			std::ostringstream xStream;
			xStream << std::fixed;
			xStream << std::setprecision(0);
			xStream << curve.points[i].x;
			std::string xValue = xStream.str();

			std::ostringstream yStream;
			yStream << curve.points[i].y;
			yStream << std::setprecision(0);
			std::string yValue = yStream.str();

			RenderText("p" + std::to_string(i) + " = (" + xValue + ", " + yValue + ")",
						10, 200 - i * 20, 0.4f, glm::vec3(1.0f, 1.0f, 1.0f));
		}
*/
		if (curve.points.size() > 1)
        {
            std::vector<glm::vec2> curve_points_ = curve.GetCurve();
            RenderLine(curve_points_);
        }
        if (curve2.points.size() > 1)
        {
            std::vector<glm::vec2> curve_points_ = curve2.GetCurve();
            RenderLine(curve_points_);
        }



		glfwSwapBuffers(window);

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && mouseHeld == false) {
			if (curve.points.size() < 10){
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				curve.RegisterPoint(xpos, ypos);
			}
			mouseHeld = true;
		}

		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
			mouseHeld = true;
		else
			mouseHeld = false;

		if (glfwGetKey(window, GLFW_KEY_C))
			curve.ClearPoints();

	}

	glfwTerminate();
	return 0;
}
