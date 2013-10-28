#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <atomic>
#include <unistd.h>
#include <iostream>
#include <thread>

class Player
{
public:
    typedef std::function<void (int)> PositionCallback;

    Player(const std::string video_file_name, const PositionCallback update_position)
    : current_frame(0)
    , paused(false)
    , update_position(update_position)
    , video_file_name(video_file_name)
    {
    }

    bool start()
    {
        cv::VideoCapture video(video_file_name);

        // check video
        if (! video.isOpened() )
        {
            std::cerr << "Could not open video '" << video_file_name << "'" << std::endl;
            return false;
        }

        // read metadata
        fps = video.get(CV_CAP_PROP_FPS);

        // read whole video - all frames
        cv::Mat frame;
        while (video.read(frame))
            frames.emplace_back(frame);

        // start thread
        thread = new std::thread(std::bind(&Player::playback_loop, this));
        return true;
    }

    void playnpause()
    {
    }

    void skip_to(int new_position_percentage)
    {
        std::cout << "skipping to:" << new_position_percentage << std::endl;
    }

private:
    double fps;
    std::vector<cv::Mat> frames;
    std::atomic<uint64_t> current_frame;
    std::atomic<bool> paused;
    std::thread * thread;
    const PositionCallback update_position;
    const std::string video_file_name;

    void playback_loop()
    {
        while (true)
        {
            if (current_frame < frames.size())
            {
                std::cout << "start video reader fps:" << fps
                          << " total frames:" << frames.size()
                          << std::endl;

                cv::imshow("video player", frames[current_frame]);
                usleep(1000000.0 / fps);
                ++current_frame;
            }
        }
    }
};

void invokeCallbackAsSkipTo(int val, void * object)
{
    static_cast<Player *>(object)->skip_to(val);
}

int main(
    int argc,
    char** argv)
{
    if (argc != 2)
    {
        std::cerr << "Not enough arguments. Usage: main <input_file>" << std::endl;
        return -1;
    }

    Player player(std::string(argv[1]), [](int new_position_percentage)
    {
        std::cout << "new video position:" << new_position_percentage << std::endl;
    });

    if (! player.start())
    {
        std::cerr << "Could not start player. Aborting" << std::endl;
        return -1;
    }

    int video_position_percentage = 0;
    cv::namedWindow("video player", cv::WINDOW_AUTOSIZE);
    using namespace std::placeholders;
    cv::createTrackbar("playback percentage", "video player",
        &video_position_percentage, 100,
        invokeCallbackAsSkipTo, &player);

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
