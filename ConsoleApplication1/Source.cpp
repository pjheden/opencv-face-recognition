#include "opencv2/core/core.hpp"
#include "opencv2/opencv.hpp"
#include <opencv2/face.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <iostream>
//#include <fstream> // Read/Write files
#include <Windows.h>//create directory

using namespace cv;
using namespace std;

int nrImgs = 25;

//UNUSED, testing only
void displayImage(string path) {
	Mat image;
	image = imread(path, CV_LOAD_IMAGE_COLOR);   // Read the file

	if (!image.data)                              // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return;
	}

	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", image);                   // Show our image inside it.

	waitKey(0);
}

/*
	Function for converting std::string to LPCWSTR
*/
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}

//UNUSED
void createTrainingPhotosOld(int person, vector<Mat> &images, vector<int> &labels) {
	cout << "Create training images for person " << person << endl;
	//create folder if it does not exist;
	string s("C:\\Users\\Julle\\OneDrive\\Documents\\Visual Studio 2015\\Projects\\ConsoleApplication1\\ConsoleApplication1\\person" + to_string(person));
	cout << "create directory " << s << endl;
	std::wstring stemp = s2ws(s);
	LPCWSTR result = stemp.c_str();
	cout << "result " << result << endl;
	CreateDirectory( result, NULL);


	/*Video capture*/
	VideoCapture cap; // open the default camera
	cap.open(0);
	if (!cap.isOpened())  // check if we succeeded
		return;

	namedWindow("Camera", 1);
	Mat picture;
	char k;
	for (;;) {
		Mat frame;
		cap >> frame;
		imshow("Camera", frame);

		k = cvWaitKey(30);
		if (k == 's') {
			break;
		}
	}
	for (int i = 0; i < nrImgs; i++)
	{
		Mat frame;
		cap >> frame;
		imshow("Camera", frame);
		frame.copyTo(picture);
		cvWaitKey(75);
		

		string s = "person" + to_string(person);
		s += "/";
		s += to_string(i) + ".jpg";
		cout << "save image: " << s << endl;
		imwrite(s, picture);
	}
	for (int i = 0; i < nrImgs; i++)
	{
		string s = "person" + to_string(person);
		s += "/";
		s += to_string(i) + ".jpg";
		cout << "read image: " << s << endl;
		images.push_back(imread(s, CV_LOAD_IMAGE_GRAYSCALE)); labels.push_back(person);
	}
}

void trainModel(vector<Mat> &images, vector<int> &labels, int person, string name) {
	Ptr<face::FaceRecognizer> model = face::createEigenFaceRecognizer();
	model->load("myModel.yml");
	model->train(images, labels);
	model->setLabelInfo(person, name);
	model->save("myModel.yml");
}

void createTrainingPhotos(int person, vector<Mat> &images, vector<int> &labels, string name) {
	cout << "Create training images for person " << person << endl;
	//NOT NEEDED BUT PROBALY USEFUL, saves images
	//create folder if it does not exist;
	string s("C:\\Users\\Julle\\OneDrive\\Documents\\Visual Studio 2015\\Projects\\ConsoleApplication1\\ConsoleApplication1\\person" + to_string(person));
	cout << "create directory " << s << endl;
	std::wstring stemp = s2ws(s);
	LPCWSTR result = stemp.c_str();
	cout << "result " << result << endl;
	CreateDirectory(result, NULL);
	//NOT NEEDED END

	/*Video capture*/
	VideoCapture cap; // open the default camera
	cap.open(0);
	if (!cap.isOpened())  // check if we succeeded
		return;

	/*Opens camera and waits for user input*/
	namedWindow("Camera", 1);
	Mat picture;
	cout << "Press s to start capturing images" << endl;
	char k;
	for (;;) {
		Mat frame;
		cap >> frame;
		imshow("Camera", frame);

		k = cvWaitKey(30);
		if (k == 's') {
			break;
		}
	}
	/* Starts taking photos*/
	for (int i = 0; i < nrImgs; i++)
	{
		Mat frame;
		cap >> frame;
		imshow("Camera", frame);
		frame.copyTo(picture);
		cvWaitKey(75);
		Mat im_gray;
		cvtColor(picture, im_gray, CV_RGB2GRAY);

		string s = "person" + to_string(person);
		s += "/";
		s += to_string(i) + ".jpg";
		imwrite(s, picture); // Not needed, saves images
		cout << "read image: " << s << endl;
		images.push_back(im_gray); labels.push_back(person);
	}
	/* Use the photos for training then saves the model*/
	trainModel(images, labels, person, name);
}

void predict(Ptr<face::FaceRecognizer> &model) {
	/*Video capture*/
	VideoCapture cap; // open the default camera
	cap.open(0);
	if (!cap.isOpened())  // check if we succeeded
		return;

	cout << "Press s to predict" << endl;
	namedWindow("Camera", 1);
	char k;
	Mat picture;
	Mat frame;
	while (1) {
		cap >> frame;
		cvtColor(frame, picture, CV_RGB2GRAY);
		imshow("Camera", frame);
		k = waitKey(1);
		if (k == 's') {
			break;
		}
	}

	Mat copy;
	picture.copyTo(copy);
	int predicted = -1;
	double confidence = 0.0;
	model->predict(copy, predicted, confidence);
	cout << "Predicted: " << predicted<< " Confidence: "<< confidence << endl;
	destroyWindow("Camera");
}

int main(int argc, const char *argv[]) {
	Ptr<face::FaceRecognizer> model;
	// These vectors hold the images and corresponding labels:
	vector<Mat> images;
	vector<int> labels;
	//int num_components = 10;
	//double threshold = 10.0;

	bool trained = false;

	while (1) {
		int n;
		cout << "Menu" << endl <<"1. Add new people" << endl 
			<< "2. Predict"<< endl << "3. Admin" << endl << "4. Exit" << endl;
		cin >> n;

		switch (n) {
		case 1:
		{
			cout << "What is your name ? " << endl;
			string name;
			cin >> name;
			cout << "What person is this ( int ) ?" << endl;
			int n;
			cin >> n;
			images.clear(); labels.clear();
			createTrainingPhotos(n, images, labels, name);

			destroyAllWindows();
			break;
		}
		case 2:
			model = face::createEigenFaceRecognizer();
			model->load("myModel.yml");
			predict(model);
			break;
		case 3:
			cout << "Enter admin password: " << endl;
			int w;
			cin >> w;
			if (w == 1234) {
				cout << "Reset Model? 1/0" << endl;
				int e;
				cin >> e;
				if (e == 1) {
					model = face::createEigenFaceRecognizer();
					model->save("myModel.yml");
				}
			}
			break;
		case 4:
			return 0;
			break;
		default:
			//do nothing
			break;
		}
	}
	
	return 0;
}