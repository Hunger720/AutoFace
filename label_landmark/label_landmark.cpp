#include <fstream>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

void onMouse(int event, int x, int y, int flags, void* params);
void saveLandmarkFile(const char* path, vector<Point> landmark);

int main()
{
	const char* filePath = "C:/Users/Hunger/Documents/MATLAB/3DMM_edges/testImages/180618/ear_landmark.txt";
	const char* imgPath  = "C:/Users/Hunger/Documents/MATLAB/3DMM_edges/testImages/180618/face.png";
	const char* title    = "label landmark";

	Mat           img = imread(imgPath);
	vector<Point> landmark;

	namedWindow(title);

	setMouseCallback(title, onMouse, (void*)&landmark);
	
	while (1)
	{
		int    radius = 2;
		int    thickness = -1;
		Scalar color(0, 255, 0);

		if (!landmark.empty())
		{
			circle(img, landmark.back(), radius, color, thickness, LINE_AA);
		}
		imshow(title, img);

		if (waitKey(10)==27)
		{
			break;
		}

		if (waitKey(10) == 's')
		{
			saveLandmarkFile(filePath, landmark);
			cout << "Saved landmark file.\n";
			break;
		}
	}

	return 0;
}


void onMouse(int event, int x, int y, int flags, void* params)
{
	vector<Point>& landmark = *(vector<Point>*)params;

	if (event == EVENT_LBUTTONDOWN)
	{
		landmark.push_back(Point(x, y));
		cout << '(' << x << ',' << y << ")\n";
	}
}


void saveLandmarkFile(const char* path, vector<Point> landmark)
{
	std::ofstream file(path, ios::out);

	for (unsigned int i = 0; i < landmark.size(); ++i)
	{
		file << landmark[i].x << ' ' << landmark[i].y << '\n';
	}

	file.close();
}