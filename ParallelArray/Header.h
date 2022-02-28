#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <future>
#include <filesystem>
#include <math.h>
#include <filesystem>
#include <list>
#include <mutex>



using namespace cv;
using namespace std;
using namespace chrono;

mutex mu;

//structure for storing images- test, training and the distences
struct Images {
    string test_name;
    Mat test_img;
    Mat grey_img;

    ~Images() {};

    friend ostream& operator<<(ostream& out, const Images& r);

};


//serial implemntation of grayscaling
void convert_to_grayscale(unsigned char* input, unsigned char* output, int start, int end, int channel, int depth) {
    if (depth > 4) {
        int j = start;
        int number_of_pixel = end;

        for (int i = start; i < number_of_pixel; i += channel) {
            int blue_value = input[i];
            int green_value = input[i + 1];
            int red_value = input[i + 2];

            int out_index = i / channel;

            output[out_index] = (int)(0.114 * blue_value + 0.587 * green_value + 0.299 * red_value);
        }
    }

    else {
        auto mid = ((start + end) / 2);
        auto left = async(launch::async, convert_to_grayscale, input, output, start, mid, channel, depth + 1);
        convert_to_grayscale(input, output, mid, end, channel, depth + 1);
    }

}
//function to calcualte the euclidian distance between the images
void calculate_distance(unsigned char* testimg, unsigned char* trainimg, int start, int end, double& total, int depth) {
    Images img;
    if (depth > 4) {

        for (int i = start; i < end; i++) {
            total = +pow((testimg[i] - trainimg[i]), 2);
        }
    }
    else {
        auto mid = ((start + end) / 2);
        auto left = async(launch::async, calculate_distance, testimg, trainimg, start, mid, ref(total), depth + 1);
        calculate_distance(testimg, trainimg, mid, end, ref(total), depth + 1);
    }
}