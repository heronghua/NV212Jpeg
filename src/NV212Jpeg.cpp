#include <iostream>
#include <fstream>
#include <filesystem>
#include <opencv2/opencv.hpp>
#include <regex>

using namespace std;
using namespace cv;
namespace fs = filesystem;

Mat NV212RGB(const string& yuvPath,int width,int height)
{
	ifstream file(yuvPath,ios::binary);
	file.seekg(0,ios::end);
	streampos fileSize = file.tellg();
	file.seekg(0,ios::beg);

	vector<unsigned char> nv21Data(fileSize);
	file.read(reinterpret_cast<char*>(nv21Data.data()), fileSize);
	file.close();

	Mat yuv(height*3/2,width,CV_8UC1,nv21Data.data());
	Mat bgr;
	cvtColor(yuv,bgr,COLOR_YUV2BGR_NV21);
	Mat bgrImage;
	bgr.convertTo(bgrImage,CV_8UC3);
	return bgrImage;

}

int main(int argc,char* argv[])
{
	if (argc< 2) {
		cerr << "Usage : ./NV212Jpeg.exe <input_folder>" << endl;
		return 1;
	}

	string inputFolder = argv[1];
	fs::path outputFolder = fs::path(inputFolder).append("nv212jpegs");
	fs::create_directories(outputFolder);

	int processCount = 0;
	//get count of nv21 files
	int nv21FileCount = 0;
	for(const auto& entry : fs::directory_iterator(inputFolder)){
		if(entry.path().extension() == ".nv21"){
			nv21FileCount ++;
		}
	}
	for(const auto& entry : fs::directory_iterator(inputFolder)){
		if(entry.path().extension() == ".nv21"){
			string inputFile = entry.path().string();
			string fileName=entry.path().filename().string();
			regex pattern(R"([A-Za-z_-]*(\d+)x(\d+))");

			smatch matches;

			if (regex_search(fileName,matches,pattern) && matches.size()==3) {
				int width = stoi(matches[1]);
				int height = stoi(matches[2]);

				//save file as jpeg
				fs::path outputFilePath = outputFolder / entry.path().filename();
				outputFilePath.replace_extension(".jpg");
				string outputFile = outputFilePath.string();

				Mat image = NV212RGB(inputFile, width, height);
				imwrite(outputFile, image);
			}
			++processCount;
			//calculate progress
			float progress = static_cast<int>(static_cast<float>(processCount)/nv21FileCount * 100.0 );
			cout << "\r" << "Converting nv21 to jpg progress : " << progress << "%" << flush;
		}
	}
	cout << " , output folder : " << outputFolder.generic_string() << endl;
	return 0;
}
