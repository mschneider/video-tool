#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <atomic>
#include <unistd.h>
#include <iostream>
#include <thread>

class Player
{
public:
    Player(const std::string video_file_name)
    : current_frame(0)
    , current_trackbar_frame(0)
    , paused(0)
    , video_file_name(video_file_name)
    {
    }

    bool start()
    {
        video = new cv::VideoCapture(video_file_name);

        // check video
        if (! video->isOpened() )
        {
            std::cerr << "Could not open video '" << video_file_name << "'" << std::endl;
            return false;
        }

        // read metadata
        fps = video->get(CV_CAP_PROP_FPS);
        total_frames = video->get(CV_CAP_PROP_FRAME_COUNT);

        cv::createTrackbar(trackbar_name, "video player",
            &current_trackbar_frame, total_frames,
            &Player::invoke_trackbar_callback, this);

        // start thread
        thread = new std::thread(std::bind(&Player::playback_loop, this));
        return true;
    }

    void toggle_pause()
    {
        paused ^= 1;
    }

    void skip_to(int new_frame)
    {
        current_frame = new_frame;
    }

    int get_total_frames() const
    {
        return total_frames;
    }

private:
    const std::string trackbar_name = "current_frame";
    double fps, total_frames;
    int current_trackbar_frame;
    std::atomic<uint64_t> current_frame;
    std::atomic<int> paused;
    std::thread * thread;
    cv::VideoCapture * video;
    const std::string video_file_name;

    void playback_loop()
    {
        const double time_between_frames = 1000000.0 / fps;
        while (true)
        {
            if (current_frame < total_frames && ! paused)
            {
                std::cout << "start video reader fps:" << fps
                          << " total frames:" << total_frames
                          << " current frame:" << current_frame
                          << " sleep:" << time_between_frames
                          << std::endl;

                cv::setTrackbarPos(trackbar_name, "video player", current_frame);
                video->set(CV_CAP_PROP_POS_FRAMES, current_frame);
                cv::Mat frame;
                if (video->read(frame))
                    cv::imshow("video player", frame);
                ++current_frame;
            }
            usleep(time_between_frames);
        }
    }

    static void invoke_trackbar_callback(int new_frame, void * player)
    {
        static_cast<Player *>(player)->skip_to(new_frame);
    }
};

void export_reverse_video(const std::string & input_file_name,
                          const std::string & output_file_name)
{
    cv::VideoCapture in(input_file_name);
    cv::VideoWriter out;
    const int width = in.get(CV_CAP_PROP_FRAME_WIDTH),
              height = in.get(CV_CAP_PROP_FRAME_HEIGHT),
              fourcc = 0; // eigentlich: in.get(CV_CAP_PROP_FOURCC), aber wir ueberschreiben das mal
    const double fps = in.get(CV_CAP_PROP_FPS);
    std::cout << "width:" << width << " height:" << height << " fourcc:" << fourcc << " fps:" << fps << std::endl;
    out.open(output_file_name, fourcc, fps, cv::Size(width, height), true);
    int current_frame = in.get(CV_CAP_PROP_FRAME_COUNT);
    while (current_frame >= 0)
    {
        cv::Mat data;
        in.set(CV_CAP_PROP_POS_FRAMES, current_frame);
        if (in.read(data))
            out << data;
        current_frame--;
    }
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

    const std::string video_file_name(argv[1]);
    const size_t extension_index = video_file_name.rfind('.');
    std::string tmp(video_file_name);
    const std::string output_file_name(tmp.replace(extension_index, std::string::npos, "_revers.avi"));
    //const std::string output_file_name(tmp.insert(extension_index, "_reverse"));

    Player player(video_file_name);

    cv::namedWindow("video player", cv::WINDOW_AUTOSIZE);

    if (! player.start())
    {
        std::cerr << "Could not start player. Aborting" << std::endl;
        return -1;
    }

    // read key inputs
    while (true)
    {
        int key_code_entered = cv::waitKey(1);
        switch (key_code_entered) {
        case ' ':
            std::cout << "pause/play" << std::endl;
            player.toggle_pause();
            break;
        case 'e':
            std::cout << "export >" << output_file_name << std::endl;
            export_reverse_video(video_file_name, output_file_name);
        case '\n':
            std::cout << "end" << std::endl;
            cv::destroyAllWindows();
            return 0;
        default:
            char key_char_entered = key_code_entered;
            //std::cout << "did not recognize key:" << key_char_entered
                      //<< " (code:" << key_code_entered << ")" << std::endl;
        }
    }
    return 0;
}
