#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream>
#include <thread>

int main(
    int argc,
    char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Not enough arguments. Usage: main <input_file>" << std::endl;
        return -1;
    }

    const std::string video_file_name(argv[1]);
    cv::VideoCapture video(video_file_name);
    if (! video.isOpened() )
    {
        std::cerr << "Could not open video '" << video_file_name << "'" << std::endl;
        return -1;
    }

    int video_position_percentage = 0;
    cv::namedWindow("video player", cv::WINDOW_AUTOSIZE);
    cv::createTrackbar("playback percentage", "video player", &video_position_percentage, 100);

    // start video thread
    std::thread video_thread([&video_file_name, &video]
    {
        const double fps = video.get(CV_CAP_PROP_FPS),
                     total_frames = video.get(CV_CAP_PROP_FRAME_COUNT);
        std::cout << "video reader for video:" << video_file_name
                  << " fps:" << fps
                  << " total frames:" << total_frames
                  << std::endl;
        cv::Mat frame;
        while (video.read(frame))
        {
            cv::imshow("video player", frame);
        }
        std::cout << "finished readig video" << std::endl;
    });

    // read key inputs
    while (true)
    {
        int key_code_entered = cv::waitKey();
        switch (key_code_entered) {
        case ' ':
            std::cout << "pause/play" << std::endl;
            break;
        case '\n':
        case -1: // window closed
            std::cout << "end" << std::endl;
            cv::destroyAllWindows();
            return 0;
        default:
            char key_char_entered = key_code_entered;
            std::cout << "did not recognize key:" << key_char_entered
                      << " (code:" << key_code_entered << ")" << std::endl;
        }
    }
    return 0;
}
