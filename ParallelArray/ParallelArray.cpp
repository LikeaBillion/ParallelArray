#include "Header.h"

//function to calculate the knn of the images- simply counts them and then prints the most common
void calculate_knn(string** distance, int kvalue) {
    //variables to hold the counts for all the different types of images to sort
    double daisy_count = 0;
    double dandelion_count = 0;
    double rose_count = 0;
    double sunflower_count = 0;
    double tulip_count = 0;
    int i;

    for (i = 0; i < kvalue; i++) {
        if (distance[1][i] == "daisy") {
            daisy_count++;
        }
        else if (distance[1][i] == "dandelion") {
            dandelion_count++;
        }
        else if (distance[1][i] == "rose") {
            rose_count++;
        }
        else if (distance[1][i] == "sunflower") {
            sunflower_count++;
        }
        else if (distance[1][i] == "tulip") {
            tulip_count++;
        }

    }
    cout << "----------------------------------------------------------" << endl;
    //cout << "Filename: " << distance[i].test_name << endl;
    cout << "Daisy with: " << daisy_count << " counts" << endl;
    cout << "Dandelion with: " << dandelion_count << " counts" << endl;
    cout << "Rose with: " << rose_count << " counts" << endl;
    cout << "Sunflower with: " << sunflower_count << " counts" << endl;
    cout << "Tulip with: " << tulip_count << " counts" << endl;
    if (max({ daisy_count,dandelion_count, rose_count, sunflower_count, tulip_count }) == daisy_count) {
        cout << "Its a daisy with: " << 100 * (daisy_count / kvalue) << " confidence" << endl;
    }
    else if (max({ daisy_count,dandelion_count, rose_count, sunflower_count, tulip_count }) == dandelion_count) {
        cout << "Its a dandelion with: " << 100 * (dandelion_count / kvalue) << " confidence" << endl;
    }
    else if (max({ daisy_count,dandelion_count, rose_count, sunflower_count, tulip_count }) == rose_count) {
        cout << "Its a rose with: " << 100 * (rose_count / kvalue) << " confidence" << endl;
    }
    else if (max({ daisy_count,dandelion_count, rose_count, sunflower_count, tulip_count }) == sunflower_count) {
        cout << "Its a sunflower with: " << 100 * (sunflower_count / kvalue) << " confidence" << endl;
    }
    else if (max({ daisy_count,dandelion_count, rose_count, sunflower_count, tulip_count }) == tulip_count) {
        cout << "Its a tulip with: " << 100 * (tulip_count / kvalue) << " confidence" << endl;
    }

}
//function to calculate label from the filename provided, just chops off anything after the last // and then before the new last //
string calculate_label(string& dir) {
    string directory;
    size_t last_slash_idx = dir.rfind('\\');
    size_t end = dir.size();
    if (std::string::npos != last_slash_idx)
    {
        directory = dir.substr(0, last_slash_idx);
        size_t second_last_slash = directory.rfind('\\');
        directory = directory.substr(second_last_slash + 1, end);
    }
    return directory;
}
//function to extract the filename from the path fo the test images
string calculate_filename(string& dir) {
    string name;
    size_t last_slash_idx = dir.rfind('\\');
    size_t end = dir.size();
    if (std::string::npos != last_slash_idx)
    {
        name = dir.substr(last_slash_idx + 1, end);

    }
    return name;
}

//function to work out the subfolders from the path provideded by the command line  
void find_subfolders(string dir, vector<string>& subfolders) {
    int label_counter = 0;
    for (const auto& entry : filesystem::directory_iterator(dir)) {
        string path_string{ entry.path().u8string() };
        subfolders.push_back(path_string);
    }

}

//bubble sort written to filter the distances by size
void bubble_sort_distance(string** distance, int values) {
    for (int i = 0; i < values - 1; i++) {
        for (int j = 0; j < (values - i - 1); j++) {
            //temp stores the values sorts then put them back in the array if needs to
            if (stoi(distance[0][j]) > stoi(distance[0][j + 1])) {
                int temp1 = stoi(distance[0][j]);
                int temp2 = stoi(distance[0][j + 1]);
                string temp3 = distance[1][j];
                string temp4 = distance[1][j + 1];
                distance[0][j] = to_string(temp2);
                distance[0][j + 1] = to_string(temp1);
                distance[1][j] = temp4;
                distance[1][j + 1] = temp3;
            }
        }
    }
}
//function to read images parallely
void read_images(vector<string>& sub_folders, vector<string>& filenames, list<Mat> &list_train_image_data,list<string> &list_labels, int start, int end, int depth) {
    Images train_img;

    if (depth > 4) {
        scoped_lock<mutex> sl(mu);
        for (int j = start; j < end; j++) {
            list_train_image_data.push_back(imread(filenames[j]));
            list_labels.push_back(calculate_label(filenames[j]));
        }
    }

    else {
        auto mid = ((start + end) / 2);
        auto left = async(launch::async, read_images, ref(sub_folders), ref(filenames), ref(list_train_image_data), ref(list_labels), start, mid, depth + 1);
        read_images(ref(sub_folders), ref(filenames), ref(list_train_image_data), ref(list_labels), mid, end, depth + 1);

    }

}




int main(int argc, char** argv)
{
    std::cout << "starting opencv...." << std::endl;
    //validating arguments
    if (argc != 3) {
        cout << "Error in arguments..\n";
        exit(-1);
    }

    //storing the passed arguments for directory and k-values
    string dir(argv[1]);
    string k_value(argv[2]);
    //string to hold the test_img entered using the cin
    string test_img_path;

    //image vectors for testing, training and distance data
    vector<Images> test_image_data;
    list<Mat> list_train_image_data;
    list<string> list_labels;

    //vectors to store filename sand test_filenames too
    vector<String> filenames;
    vector<String> test_img_filenames;

    //vector to hold all the subfolders from the main directory
    vector<String> sub_folders;

    //staring timing
    auto start = steady_clock::now();

    //cin the test_img path
    cout << "Input test directory: " << endl;
    cin >> test_img_path;

    //function calls to find all the subfolders
    find_subfolders(dir, ref(sub_folders));
    //iterates through all the subfolders adding to the image vector with all the training data and their labels
    for (int i = 0; i < sub_folders.size(); i++) {
        glob(sub_folders[i], filenames);
        read_images(ref(sub_folders), ref(filenames), ref(list_train_image_data), ref(list_labels), 0, filenames.size(), 0);
    }
    
    //interates through all the test_imgs add puts them into the image vector. Along with the test_name to be used later
    Images test_img;
    glob(test_img_path, test_img_filenames);
    for (int i = 0; i < test_img_filenames.size(); i++) {
        test_img.test_img = (imread(test_img_filenames[i]));
        test_img.test_name = calculate_filename(test_img_filenames[i]);
        test_image_data.push_back(test_img);
    }


    //new image vector for distances, this is used so the vector can be cleared after interations to insure that 1 images distances don't bleed into other images data
    Images distance_img;
    //interates through the test data and then the train data for that- calculates distance then the knn before clearing the vector
    for (int i = 0; i < test_image_data.size(); i++) {

        //storing the current testing image as variable for easy access
        auto test_image = test_image_data[i].test_img;

        //allocating memory for the test_input and train_outputs
        unsigned char* test_input = (unsigned char*)test_image.data;
        unsigned char* test_output = new unsigned char[test_image.size().width * test_image.size().height];
        //count for the max number of pixels
        const int total_number_of_pixels = test_image.rows * test_image.cols * test_image.channels();
        const int train_size = list_train_image_data.size();
        convert_to_grayscale(test_input, test_output, 0, total_number_of_pixels, test_image.channels(),0);
        //creating the 2d array with as many colomns as is necessary for the task

        string** distance = new string * [2];
        for (int i = 0; i < 2; ++i) {
            distance[i] = new string[train_size];
        }


        for (int j = 0; j < train_size; j++) {

            //pulling current image from the list
            auto lt = list_train_image_data.begin();
            std::advance(lt, j);

            //storing the current training image as variable for easy access
            Mat train_img = *lt;
            //storing total which is used to calculate distance
            double total = 0;

            //allocating memory for the train_input and train_outputs
            unsigned char* train_input = (unsigned char*)train_img.data;
            unsigned char* train_output = new unsigned char[train_img.size().width * train_img.size().height];

            //calcuating greyscale
            convert_to_grayscale(train_input, train_output, 0, total_number_of_pixels, train_img.channels(),0);

            //pulling from list labels
            auto ll = list_labels.begin();
            std::advance(ll, j);

            calculate_distance(test_output, train_output, 0, (total_number_of_pixels / 3), ref(total), 0);
            //adding to the calculate distance array- for distance
            distance[0][j] = to_string(sqrt(total));
            //adding to the calculate distance array- for labels
            distance[1][j] = *ll;


            

            delete[] train_output;
        }

        //calling the bubble sort
        bubble_sort_distance(distance, train_size);
        //for testing output of distances from the array
        /*for (int h = 0; h < train_size; h++) {
            cout << h << ": " << distance[0][h] << endl;
        }*/

        calculate_knn(distance, stoi(k_value));
        
        //deleting to avoid memory leaks 
        delete[] test_output;
        delete[] distance;
    }

    //ending timing
    auto end = steady_clock::now();
    //calculating duration
    double duration_p = duration_cast<milliseconds> (end - start).count();
    cout << "----------------------------------------------------------" << endl;
    //outputing parallel time
    cout << "Paralell time: " << duration_p/1000 << endl;

    return 0;
}
