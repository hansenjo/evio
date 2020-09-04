/**
 * Copyright (c) 2019, Jefferson Science Associates
 *
 * Thomas Jefferson National Accelerator Facility
 * Data Acquisition Group
 *
 * 12000, Jefferson Ave, Newport News, VA 23606
 * Phone : (757)-269-7100
 *
 * @date 07/05/2019
 * @author timmer
 */


#include <functional>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <chrono>
#include <thread>
#include <memory>
#include <regex>
#include <iterator>

#ifndef __APPLE__
#include <experimental/filesystem>
#endif

#include "evio.h"


using namespace std;

#ifndef __APPLE__
namespace fs = std::experimental::filesystem;
#endif

namespace evio {


    class ReadWriteTest {


    public:

        uint32_t *int1;
        uint8_t *byte1;
        uint16_t *short1;
        uint64_t *long1;
        float *float1;
        double *double1;

        std::vector<uint32_t> intVec;
        std::vector<uint8_t> byteVec;
        std::vector<uint16_t> shortVec;
        std::vector<uint64_t> longVec;
        std::vector<float> floatVec;
        std::vector<double> doubleVec;
        std::vector<std::string> stringsVec;

        int dataElementCount = 3;
        int skip = 0;
        int bufSize = 200000;

        bool oldEvio = false;
        bool useBuf = false;

        std::shared_ptr<ByteBuffer> buffer;

        // files for input & output
        std::string writeFileName1 = "./compactEvioBuild.ev";
        std::string writeFileName0 = "./compactEvioBuildOld.ev";
        std::string writeFileName2 = "./rawEvioStructure.ev";

        ByteOrder order{ByteOrder::ENDIAN_LOCAL};

        std::string dictionary;


        ReadWriteTest() {

            uint16_t tag = 1;
            uint8_t num = 1;
            buffer = std::make_shared<ByteBuffer>(bufSize);
            buffer->order(order);


            std::cout << "Running with:" << std::endl;
            std::cout << " data elements = " << dataElementCount << std::endl;
            std::cout << "       bufSize = " << bufSize << std::endl;
            std::cout << "        useBuf = " << useBuf << std::endl;
            std::cout << "      old evio = " << oldEvio << std::endl;


            setDataSize(dataElementCount);

            //            if (oldEvio) {
            //                createObjectEvents(tag, num);
            //            }
            //            else {
            //                createCompactEvents(tag, num);
            //            }

            std::stringstream ss;

            ss << "<xmlDict>\n" <<
               "  <bank name=\"HallD\"             tag=\"6-8\"  type=\"bank\" >\n" <<
               "      <description format=\"New Format\" >hall_d_tag_range</description>\n" <<
               "      <bank name=\"DC(%t)\"        tag=\"6\" num=\"4\" >\n" <<
               "          <leaf name=\"xpos(%n)\"  tag=\"6\" num=\"5\" />\n" <<
               "          <bank name=\"ypos(%n)\"  tag=\"6\" num=\"6\" />\n" <<
               "      </bank >\n" <<
               "      <bank name=\"TOF\"     tag=\"8\" num=\"0\" >\n" <<
               "          <leaf name=\"x\"   tag=\"8\" num=\"1\" />\n" <<
               "          <bank name=\"y\"   tag=\"8\" num=\"2\" />\n" <<
               "      </bank >\n" <<
               "      <bank name=\"BCAL\"      tag=\"7\" >\n" <<
               "          <leaf name=\"x(%n)\" tag=\"7\" num=\"1-3\" />\n" <<
               "      </bank >\n" <<
               "  </bank >\n" <<
               "  <dictEntry name=\"JUNK\" tag=\"5\" num=\"0\" />\n" <<
               "  <dictEntry name=\"SEG5\" tag=\"5\" >\n" <<
               "       <description format=\"Old Format\" >tag 5 description</description>\n" <<
               "  </dictEntry>\n" <<
               "  <bank name=\"Rangy\" tag=\"75 - 78\" >\n" <<
               "      <leaf name=\"BigTag\" tag=\"76\" />\n" <<
               "  </bank >\n" <<
               "</xmlDict>";

            dictionary = ss.str();

            std::cout << "Const: dictionary = " << dictionary << std::endl;


        }


        void setDataSize(int elementCount) {

            int1 = new uint32_t[elementCount];
            byte1 = new uint8_t[elementCount];
            short1 = new uint16_t[elementCount];
            long1 = new uint64_t[elementCount];
            float1 = new float[elementCount];
            double1 = new double[elementCount];

            intVec.reserve(elementCount);
            byteVec.reserve(elementCount);
            shortVec.reserve(elementCount);
            longVec.reserve(elementCount);
            floatVec.reserve(elementCount);
            doubleVec.reserve(elementCount);
            stringsVec.reserve(elementCount);

            for (int i = 0; i < elementCount; i++) {
                int1[i] = i + 1;
                byte1[i] = (uint8_t) ((i + 1) % std::numeric_limits<uint8_t>::max());
                short1[i] = (uint16_t) ((i + 1) % std::numeric_limits<uint16_t>::max());
                long1[i] = i + 1;
                float1[i] = (float) (i + 1);
                double1[i] = (double) (i + 1);

                intVec.push_back(i + 1);
                byteVec.push_back((uint8_t) ((i + 1) % std::numeric_limits<uint8_t>::max()));
                shortVec.push_back((uint16_t) ((i + 1) % std::numeric_limits<uint16_t>::max()));
                longVec.push_back(i + 1);
                floatVec.push_back(static_cast<float>(i + 1));
                doubleVec.push_back(static_cast<double>(i + 1));

                stringsVec.push_back("0x" + std::to_string(i + 1));
            }
        }



        std::shared_ptr<ByteBuffer> generateEvioBuffer(ByteOrder & order, uint16_t tag, uint8_t num) {

            CompactEventBuilder builder(buffer);

            // add top/event level bank of banks
            builder.openBank(tag, DataType::BANK, num);

            // add bank of banks
            builder.openBank(tag + 1, DataType::BANK, num + 1);

            // add bank of ints
            builder.openBank(tag + 2, DataType::UINT32, num + 2);
            builder.addIntData(int1, dataElementCount);
            builder.closeStructure();

            // add bank of bytes
            builder.openBank(tag + 3, DataType::UCHAR8, num + 3);
            builder.addByteData(byte1, dataElementCount);
            builder.closeStructure();

            // add bank of shorts
            builder.openBank(tag + 4, DataType::USHORT16, num + 4);
            builder.addShortData(short1, dataElementCount);
            builder.closeStructure();

            // add bank of longs
            builder.openBank(tag + 40, DataType::ULONG64, num + 40);
            builder.addLongData(long1, dataElementCount);
            builder.closeStructure();

            // add bank of banks
            builder.openBank(tag + 1000, DataType::BANK, num + 1000);

            // add bank of shorts
            builder.openBank(tag + 1200, DataType::USHORT16, num + 1200);
            builder.addShortData(short1, dataElementCount);
            builder.closeStructure();

            builder.closeStructure();

            // add bank of floats
            builder.openBank(tag + 5, DataType::FLOAT32, num + 5);
            builder.addFloatData(float1, dataElementCount);
            builder.closeStructure();

            // add bank of doubles
            builder.openBank(tag + 6, DataType::DOUBLE64, num + 6);
            builder.addDoubleData(double1, dataElementCount);
            builder.closeStructure();

            // add bank of strings
            builder.openBank(tag + 7, DataType::CHARSTAR8, num + 7);
            builder.addStringData(stringsVec);
            builder.closeStructure();

            // add bank of composite data
            builder.openBank(tag + 100, DataType::COMPOSITE, num + 100);

            // Now create some COMPOSITE data
            // Format to write 1 int and 1 float a total of N times
            std::string format = "N(I,F)";

            CompositeData::Data myData;
            myData.addN(2);
            myData.addInt(1);
            myData.addFloat(1.0F);
            myData.addInt(2);
            myData.addFloat(2.0F);

            // Create CompositeData object
            auto cData = CompositeData::getInstance(format, myData, 1, 1, 1);
            std::vector<std::shared_ptr<CompositeData>> cDataVec;
            cDataVec.push_back(cData);

            // Add to bank
            builder.addCompositeData(cDataVec);
            builder.closeStructure();

            // add bank of segs
            builder.openBank(tag + 14, DataType::SEGMENT, num + 14);

            // add seg of ints
            builder.openSegment(tag + 8, DataType::INT32);
            builder.addIntData(int1, dataElementCount);
            builder.closeStructure();

            // add seg of bytes
            builder.openSegment(tag + 9, DataType::CHAR8);
            builder.addByteData(byte1, dataElementCount);
            builder.closeStructure();

            // add seg of shorts
            builder.openSegment(tag + 10, DataType::SHORT16);
            builder.addShortData(short1, dataElementCount);
            builder.closeStructure();

            // add seg of longs
            builder.openSegment(tag + 40, DataType::LONG64);
            builder.addLongData(long1, dataElementCount);
            builder.closeStructure();

            // add seg of floats
            builder.openSegment(tag + 11, DataType::FLOAT32);
            builder.addFloatData(float1, dataElementCount);
            builder.closeStructure();

            // add seg of doubles
            builder.openSegment(tag + 12, DataType::DOUBLE64);
            builder.addDoubleData(double1, dataElementCount);
            builder.closeStructure();

            // add seg of strings
            builder.openSegment(tag + 13, DataType::CHARSTAR8);
            builder.addStringData(stringsVec);
            builder.closeStructure();

            builder.closeStructure();


            // add bank of tagsegs
            builder.openBank(tag + 15, DataType::TAGSEGMENT, num + 15);

            // add tagseg of ints
            builder.openTagSegment(tag + 16, DataType::UINT32);
            builder.addIntData(int1, dataElementCount);
            builder.closeStructure();

            // add tagseg of bytes
            builder.openTagSegment(tag + 17, DataType::UCHAR8);
            builder.addByteData(byte1, dataElementCount);
            builder.closeStructure();

            // add tagseg of shorts
            builder.openTagSegment(tag + 18, DataType::USHORT16);
            builder.addShortData(short1, dataElementCount);
            builder.closeStructure();

            // add tagseg of longs
            builder.openTagSegment(tag + 40, DataType::ULONG64);
            builder.addLongData(long1, dataElementCount);
            builder.closeStructure();

            // add tagseg of floats
            builder.openTagSegment(tag + 19, DataType::FLOAT32);
            builder.addFloatData(float1, dataElementCount);
            builder.closeStructure();

            // add tagseg of doubles
            builder.openTagSegment(tag + 20, DataType::DOUBLE64);
            builder.addDoubleData(double1, dataElementCount);
            builder.closeStructure();

            // add tagseg of strings
            builder.openTagSegment(tag + 21, DataType::CHARSTAR8);
            builder.addStringData(stringsVec);
            builder.closeStructure();

            builder.closeAll();

            // Make this call to set proper pos & lim
            return builder.getBuffer();
        }




        void writeFile(string finalFilename) {

            // Variables to track record build rate
            double freqAvg;
            long totalC = 0;
            long loops = 3;

            uint8_t firstEvent[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            uint32_t firstEventLen = 10;
            bool addTrailerIndex = true;

            ByteOrder order = ByteOrder::ENDIAN_LOCAL;
            //Compressor::CompressionType compType = Compressor::GZIP;
            Compressor::CompressionType compType = Compressor::UNCOMPRESSED;

            // Possible user header data
            uint8_t * userHdr = new uint8_t[10];
            for (uint8_t i = 0; i < 10; i++) {
                userHdr[i] = i;
            }


//            Writer::Writer(const HeaderType & hType, const ByteOrder & order,
//            uint32_t maxEventCount, uint32_t maxBufferSize,
//            const std::string & dictionary, uint8_t* firstEvent, uint32_t firstEventLen,
//            const Compressor::CompressionType & compType, bool addTrailerIndex) {


            // Create files
            Writer writer(HeaderType::EVIO_FILE, order, 0, 0,
                          dictionary, firstEvent, firstEventLen, compType, addTrailerIndex);
            writer.open(finalFilename);
            //writer.open(finalFilename, userHdr, 10);
            cout << "Past creating Writer object" << endl;

            // Calling the following method makes a shared pointer out of dataArray, so don't delete
            ByteBuffer dataBuffer(20);
            for (int i=0; i < 10; i++) {
                dataBuffer.putShort(i);
            }
            dataBuffer.flip();

            //cout << "Data buffer ->\n" << dataBuffer.toString() << endl;

            // Create an evio bank of ints
            auto evioDataBuf = generateEvioBuffer(order, 0, 0);
            // Create node from this buffer
            std::shared_ptr<EvioNode> node = EvioNode::extractEventNode(evioDataBuf,0,0,0);

            auto t1 = std::chrono::high_resolution_clock::now();

            while (true) {
                // random data array
                //writer.addEvent(dataArray, 0, 20);
                cout << "add event of len = " << dataBuffer.remaining() << endl;
                writer.addEvent(dataBuffer);

                //cout << int(20000000 - loops) << endl;
                totalC++;

                if (--loops < 1) break;
            }

            cout << "add event of node,  data type = " << node->getDataTypeObj().toString() << ", bytes = " << node->getTotalBytes() << endl;
            writer.addEvent(*node.get());

            auto t2 = std::chrono::high_resolution_clock::now();
            auto deltaT = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

            freqAvg = (double) totalC / deltaT.count() * 1000;

            cout << "Time = " << deltaT.count() << " msec,  Hz = " << freqAvg << endl;
            cout << "Finished all loops, count = " << totalC << endl;

            //------------------------------
            // Add entire record at once
            //------------------------------

            RecordOutput recOut(order);
            ByteBuffer dataBuffer2(40);
            for (int i=0; i < 20; i++) {
                dataBuffer2.putShort(i);
            }
            dataBuffer2.flip();
            cout << "add entire record (containing " << dataBuffer2.remaining() << " bytes of data) " << endl;
            recOut.addEvent(dataBuffer2.array(), 40, 0);
            writer.writeRecord(recOut);

            //------------------------------

            //writer1.addTrailer(true);
            //writer.addTrailerWithIndex(addTrailerIndex);
            cout << "Past write" << endl;

            writer.close();
            cout << "Past close" << endl;

            // Doing a diff between files shows they're identical!

            cout << "Finished writing file " << finalFilename << " now read it" << endl;

            Util::printBytes(finalFilename, 0, 2000, "File bytes");

            delete[] userHdr;
        }


        void writeFileMT(string fileName) {

            // Variables to track record build rate
            double freqAvg;
            long totalC = 0;
            long loops = 3;


            string dictionary = "This is a dictionary";
            //dictionary = "";
            uint8_t firstEvent[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
            uint32_t firstEventLen = 10;
            bool addTrailerIndex = true;
            ByteOrder order = ByteOrder::ENDIAN_LITTLE;
            //Compressor::CompressionType compType = Compressor::GZIP;
            Compressor::CompressionType compType = Compressor::UNCOMPRESSED;

            // Possible user header data
            auto userHdr = new uint8_t[10];
            for (uint8_t i = 0; i < 10; i++) {
                userHdr[i] = i;
            }

            // Create files
            string finalFilename1 = fileName;
            WriterMT writer1(HeaderType::EVIO_FILE, order, 0, 0,
                             dictionary, firstEvent, 10, compType, 2, addTrailerIndex, 16);
            writer1.open(finalFilename1, userHdr, 10);
            cout << "Past creating writer1" << endl;

            uint8_t dataArray[] = {0,1,2,3,4,5,6,7,8,9};
            // Calling the following method makes a shared pointer out of dataArray, so don't delete
            ByteBuffer dataBuffer(dataArray, 20);

            // Create an evio bank of ints
            auto evioDataBuf = generateEvioBuffer(order, 0, 0);
            // Create node from this buffer
            std::shared_ptr<EvioNode> node = EvioNode::extractEventNode(evioDataBuf,0,0,0);


            auto t1 = std::chrono::high_resolution_clock::now();

            while (true) {
                // random data dataArray
                //writer1.addEvent(buffer, 0, 26);
                writer1.addEvent(dataBuffer);

                //cout << int(20000000 - loops) << endl;
                totalC++;

                if (--loops < 1) break;
            }

            writer1.addEvent(*node.get());

            auto t2 = std::chrono::high_resolution_clock::now();
            auto deltaT = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

            freqAvg = (double) totalC / deltaT.count() * 1000;

            cout << "Time = " << deltaT.count() << " msec,  Hz = " << freqAvg << endl;
            cout << "Finished all loops, count = " << totalC << endl;

            //------------------------------
            // Add entire record at once
            //------------------------------

            RecordOutput recOut(order);
            recOut.addEvent(dataArray, 0, 26);
            writer1.writeRecord(recOut);

            //------------------------------

            //writer1.addTrailer(true);
            writer1.addTrailerWithIndex(addTrailerIndex);
            cout << "Past write" << endl;

            writer1.close();
            cout << "Past close" << endl;

            // Doing a diff between files shows they're identical!

            cout << "Finished writing file " << fileName << ", now read it in" << endl;

            //delete[] dataArray;
            delete[] userHdr;
        }



//
//        string eventWriteFileMT(const string & filename) {
//
//            // Variables to track record build rate
//            double freqAvg;
//            long totalC = 0;
//            long loops = 6;
//
//
//            string dictionary = "";
//
//            uint8_t firstEvent[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
//            uint32_t firstEventLen = 10;
//            bool addTrailerIndex = true;
//            ByteOrder order = ByteOrder::ENDIAN_BIG;
//            //Compressor::CompressionType compType = Compressor::GZIP;
//            Compressor::CompressionType compType = Compressor::UNCOMPRESSED;
//
//            // Create files
//            string directory;
//            uint32_t runNum = 123;
//            uint64_t split = 000000; // 2 MB
//            uint32_t maxRecordSize = 0; // 0 -> use default
//            uint32_t maxEventCount = 2; // 0 -> use default
//            bool overWriteOK = true;
//            bool append = true;
//            uint32_t streamId = 3;
//            uint32_t splitNumber = 2;
//            uint32_t splitIncrement = 1;
//            uint32_t streamCount = 2;
//            uint32_t compThreads = 2;
//            uint32_t ringSize = 16;
//            uint32_t bufSize = 1;
//
//            EventWriter writer(filename, directory, "runType",
//                               runNum, split, maxRecordSize, maxEventCount,
//                               order, dictionary, overWriteOK, append,
//                               nullptr, streamId, splitNumber, splitIncrement, streamCount,
//                               compType, compThreads, ringSize, bufSize);
//
//            //            string firstEv = "This is the first event";
//            //            ByteBuffer firstEvBuf(firstEv.size());
//            //            Util::stringToASCII(firstEv, firstEvBuf);
//            //            writer.setFirstEvent(firstEvBuf);
//
//
//            //            //uint8_t *dataArray = generateSequentialInts(100, order);
//            //            uint8_t *dataArray = generateSequentialShorts(13, order);
//            //            // Calling the following method makes a shared pointer out of dataArray, so don't delete
//            //            ByteBuffer dataBuffer(dataArray, 26);
//
//            //  When appending, it's possible the byte order gets switched
//            order = writer.getByteOrder();
//
//
//            // Create an evio bank of ints
//            auto evioDataBuf = generateEvioBuffer(order, 0, 0);
//            // Create node from this buffer
//            std::shared_ptr<EvioNode> node = EvioNode::extractEventNode(evioDataBuf,0,0,0);
//
//
//            auto t1 = std::chrono::high_resolution_clock::now();
//
//            while (true) {
//                cout << "Write event ~ ~ ~ " << endl;
//                // event in evio format
//                writer.writeEvent(*(evioDataBuf.get()));
//
//                totalC++;
//
//                if (--loops < 1) break;
//            }
//
//            //writer.addEvent(*node.get());
//
//            //            auto t2 = std::chrono::high_resolution_clock::now();
//            //            auto deltaT = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
//            //
//            //            freqAvg = (double) totalC / deltaT.count() * 1000;
//            //
//            //            cout << "Time = " << deltaT.count() << " msec,  Hz = " << freqAvg << endl;
//            //            cout << "Finished all loops, count = " << totalC << endl;
//
//            //------------------------------
//
//            //cout << "Past write, sleep for 2 sec ..., then close" << endl;
//            //std::this_thread::sleep_for(2s);
//
//            writer.close();
//            cout << "Past close" << endl;
//
//            // Doing a diff between files shows they're identical!
//
//            cout << "Finished writing file " << writer.getCurrentFilename() << ", now read it in" << endl;
//            return writer.getCurrentFilename();
//
//        }






        void readFile(string finalFilename) {

            Reader reader1(finalFilename);
            ByteOrder order = reader1.getByteOrder();

            int32_t evCount = reader1.getEventCount();
            cout << "Read in file " << finalFilename << ", got " << evCount << " events" << endl;

            string dict = reader1.getDictionary();
            cout << "   Got dictionary = " << dict << endl;

            shared_ptr<uint8_t> & pFE = reader1.getFirstEvent();
            if (pFE != nullptr) {
                int32_t feBytes = reader1.getFirstEventSize();
                cout << "   First Event bytes = " << feBytes << endl;
                cout << "   First Event values = " << endl << "   ";
                for (int i = 0; i < feBytes; i++) {
                    cout << (uint32_t) ((pFE.get())[i]) << ",  ";
                }
                cout << endl << endl;
            }

            cout << "Print out regular events:" << endl;
            uint32_t byteLen;

            for (int i=0; i < reader1.getEventCount(); i++) {
                shared_ptr<uint8_t> data = reader1.getEvent(i, &byteLen);
                Util::printBytes(data.get(), byteLen, "  Event #" + std::to_string(i));

//                uint32_t wordLen = byteLen / 2;
//                if (data != nullptr) {
//                    short *pData = reinterpret_cast<short *>(data.get());
//                    cout << "   Event #0, values =" << endl << "   ";
//                    for (int i = 0; i < wordLen; i++) {
//                        if (order.isLocalEndian()) {
//                            cout << *pData << ",  ";
//                        }
//                        else {
//                            cout << SWAP_16(*pData) << ",  ";
//                        }
//                        pData++;
//                        if ((i + 1) % 5 == 0) cout << endl;
//                    }
//                    cout << endl;
//                }
            }

        }


        void readFile2(string finalFilename) {

            Reader reader1(finalFilename);
            ByteOrder order = reader1.getByteOrder();

            int32_t evCount = reader1.getEventCount();
            cout << "Read in file " << finalFilename << ", got " << evCount << " events" << endl;

            string dict = reader1.getDictionary();
            cout << "   Got dictionary = " << dict << endl;

            shared_ptr<uint8_t> & pFE = reader1.getFirstEvent();
            if (pFE != nullptr) {
                int32_t feBytes = reader1.getFirstEventSize();
                auto feData = reinterpret_cast<char *>(pFE.get());
                string feString(feData, feBytes);

                cout << "First event = " << feString << endl;
            }

            cout << "reader.getEvent(0)" << endl;
            uint32_t byteLen;
            shared_ptr<uint8_t> data = reader1.getEvent(0, &byteLen);
            cout << "got event" << endl;

            uint32_t wordLen = byteLen/4;
            if (data != nullptr) {
                uint32_t *pData = reinterpret_cast<uint32_t *>(data.get());
                cout <<  "   Event #0, values =" << endl << "   ";
                for (int i = 0; i < wordLen; i++) {
                    if (order.isLocalEndian()) {
                        cout << *pData << ",  ";
                    }
                    else {
                        cout << SWAP_32(*pData) <<",  ";
                    }
                    pData++;
                    if ((i+1)%5 == 0) cout << endl;
                }
                cout << endl;
            }

        }



        void convertor() {
            string filenameIn("/dev/shm/hipoTest1.evio");
            string filenameOut("/dev/shm/hipoTestOut.evio");
            try {
                Reader reader(filenameIn);
                uint32_t nevents = reader.getEventCount();

                cout << "     OPENED FILE " << filenameOut << " for writing " << nevents << " events to " << filenameOut << endl;
                Writer writer(filenameOut, ByteOrder::ENDIAN_LITTLE, 10000, 8*1024*1024);
                //writer.setCompressionType(Compressor::LZ4);

                for (int i = 0; i < nevents; i++) {
                    cout << "     Try getting EVENT # " << i << endl;
                    uint32_t eventLen;
                    shared_ptr<uint8_t> pEvent = reader.getEvent(i, &eventLen);
                    cout << "     Got event " << i << endl;
                    cout << "     Got event len = " << eventLen << endl;

                    writer.addEvent(pEvent.get(), eventLen);
                }
                cout << "     converter END" << endl;
                writer.close();
            } catch (EvioException & ex) {
                cout << ex.what() << endl;
            }

        }


    };


}




int main(int argc, char **argv) {


    string filename   = "./hipoTest.evio";
    string filenameMT = "./hipoTestMT.evio";
    //    string filename   = "/dev/shm/hipoTest.evio";
    //    string filenameMT = "/dev/shm/hipoTestMT.evio";

    evio::ReadWriteTest tester;

    // Write files
    tester.writeFile(filename);
    //tester.writeFileMT(filenameMT);

    // Read files just written
    tester.readFile(filename);
    cout << endl << endl << "----------------------------------------" << endl << endl;
   // tester.readFile(filenameMT);

    return 0;
}
