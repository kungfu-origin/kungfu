//
// Created by qlu on 2019/3/18.
//

#include <spdlog/spdlog.h>

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
        long start_nano  = kungfu::yijinjing::parseTime(start_time.c_str(), TIME_FORMAT);
        long end_nano = kungfu::yijinjing::parseTime(end_time.c_str(), TIME_FORMAT);
        kungfu::yijinjing::JournalReaderPtr reader = kungfu::yijinjing::JournalReader::create(folder_, name_, start_nano, PRINTER_NAME);
        int i = 0;
        do
        {
            kungfu::yijinjing::FramePtr frame = reader->getNextFrame();
            while (frame.get() != nullptr && end_nano > frame->getNano())
            {
                short msgType = frame->getMsgType();
                bool toPrint = true;
                if (toPrint)
                {
                    std::cout << "[" << i++ << "]"
                              << " (st)" << (short)frame->getStatus()
                              << " (so)" << frame->getSource()
                              << " (na)" << frame->getNano()
                              << " (en)" << frame->getExtraNano();
                    if (to_time_visual_)
                    {
                        std::cout << " (vn)" << kungfu::yijinjing::parseNano(frame->getNano(), TIME_FORMAT);
                    }
                    std::cout << " (fl)" << frame->getFrameLength()
                              << " (dl)" << frame->getDataLength()
                              << " (hl)" << frame->getHeaderLength()
                              << " (hs)" << frame->getHashCode()
                              << " (mt)" << frame->getMsgType()
                              << " (lf)" << (short)frame->getLastFlag()
                              << " (id)" << frame->getRequestId()
                              << " (er)" << frame->getErrorId();
                    if (frame->getErrorMsg() != nullptr)
                    {
                        std::cout << " (em)" << frame->getErrorMsg();
                    }
                    if (length_ > 0)
                    {
                        std::cout << " (cn)" << string((char*)frame->getData(), std::min(length_, frame->getDataLength()));
                    }
                    if (to_verify_)
                    {
                        kungfu::yijinjing::FH_TYPE_HASHNM hash = kungfu::yijinjing::MurmurHash2(frame->getData(), frame->getDataLength(), kungfu::yijinjing::HASH_SEED);
                        if (hash != frame->getHashCode())
                        {
                            std::cerr << std::endl << std::endl
                                      << "Hash Code mismatch: "
                                      << "[frame] " << frame->getHashCode()
                                      << "  [actual] " << hash << std::endl;
                            return;
                        }
                        else
                        {
                            std::cout << " (*)";
                        }
                    }
                    std::cout << std::endl;
                    if (need_detail_)
                    {
                        print_data(frame->getData(), (kungfu::MsgType)msgType);
                    }
                }
                frame = reader->getNextFrame();
            }
        }
        while (keep);
    }
}
