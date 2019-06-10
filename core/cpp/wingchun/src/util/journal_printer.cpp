//
// Created by qlu on 2019/3/18.
//

#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/io.h>
#include <kungfu/yijinjing/util/util.h>
#include <kungfu/wingchun/util/journal_printer.h>

namespace kungfu
{
    void JournalPrinter::print_data(const void* data, kungfu::MsgType msg_type)
    {
        try
        {
            nlohmann::json j;
            switch (msg_type)
            {
                case kungfu::MsgType::Quote:
                {
                    j = *(const kungfu::journal::Quote *) data;
                    break;
                }
                case kungfu::MsgType::OrderInput:
                {
                    j = *(const kungfu::journal::OrderInput *) data;
                    break;
                }
                case kungfu::MsgType::Order:
                {
                    j = *(const kungfu::journal::Order *) data;;
                    break;
                }
                case kungfu::MsgType::Trade:
                {
                    j = *(const kungfu::journal::Trade *) data;
                    break;
                }
                case kungfu::MsgType::AlgoOrderInput:
                {
                    j = nlohmann::json::parse(std::string((char *) data));
                    break;
                }
                default:
                {
                    break;
                }
            }
            std::cout << j << std::endl;
        }
        catch (std::exception& e)
        {
            std::cerr << "parse data error, data " << (char*)data << " exception: " <<  e.what() << std::endl;
        }
    }

    void JournalPrinter::run(std::string start_time, std::string end_time, bool keep)
    {
//        long start_nano  = kungfu::yijinjing::time::strptime(start_time.c_str(), TIME_FORMAT);
//        long end_nano = kungfu::yijinjing::time::strptime(end_time.c_str(), TIME_FORMAT);
//        auto reader = desk.open_reader(kungfu::yijinjing::data::mode::LIVE, kungfu::yijinjing::data::category::MD, "xtp", name_);
//        reader->seek_to_time(start_nano);
//        int i = 0;
//        do
//        {
//            yijinjing::journal::frame &frame = reader->current_frame();
//            while (frame.has_data() && end_nano > frame.gen_time())
//            {
//                int16_t msgType = frame.msg_type();
//                bool toPrint = true;
//                if (toPrint)
//                {
//                    std::cout << "[" << i++ << "]"
//                              << " (so)" << frame.source()
//                              << " (na)" << frame.gen_time()
//                              << " (en)" << frame.trigger_time();
//                    if (to_time_visual_)
//                    {
//                        std::cout << " (vn)" << kungfu::yijinjing::time::strftime(frame.gen_time(), TIME_FORMAT);
//                    }
//                    std::cout << " (fl)" << frame.frame_length()
//                              << " (dl)" << frame.data_length()
//                              << " (hl)" << frame.header_length()
//                              << " (hs)" << frame.hashcode()
//                              << " (mt)" << frame.msg_type();
//                    if (to_verify_)
//                    {
//                        int32_t hash = kungfu::yijinjing::util::hash(reinterpret_cast<void *>(frame.address()), frame.data_length(), KUNGFU_HASH_SEED);
//                        if (hash != frame.hashcode())
//                        {
//                            std::cerr << std::endl << std::endl
//                                      << "Hash Code mismatch: "
//                                      << "[frame] " << frame.hashcode()
//                                      << "  [actual] " << hash << std::endl;
//                            return;
//                        }
//                        else
//                        {
//                            std::cout << " (*)";
//                        }
//                    }
//                    std::cout << std::endl;
//                }
//                reader->seek_next();
//            }
//        }
//        while (keep);
    }
}
