//
// Created by Carl Timmer on 2019-07-05.
//

#ifndef EVIO_6_0_READWRITETEST_H
#define EVIO_6_0_READWRITETEST_H


#include <cstdint>
#include "ByteBuffer.h"
#include "Writer.h"
#include "WriterMT.h"

class ReadWriteTest {



public:


    static uint8_t * generateBuffer(int size){
        auto buffer = new uint8_t[size];
        for(int i = 0; i < size; i++){
            buffer[i] = i;
        }
        return buffer;
    }



    static void testStreamRecord() {

            // Variables to track record build rate
            double freqAvg;
            long t1, t2, deltaT, totalC=0;
            // Ignore the first N values found for freq in order
            // to get better avg statistics. Since the JIT compiler in java
            // takes some time to analyze & compile code, freq may initially be low.
            long ignore = 10000;
            long loops  = 2000000;

            // Create file
            auto writer = Writer();
            writer.getRecordHeader().setCompressionType(Compressor::CompressionType);
            writer.open("/daqfs/home/timmer/exampleFile.v6.evio");


        uint8_t* buffer = generateBuffer(400);

            t1 = System.currentTimeMillis();

            while (true) {
                // random data array
                writer.addEvent(buffer);

//System.out.println(""+ (20000000 - loops));
                // Ignore beginning loops to remove JIT compile time
                if (ignore-- > 0) {
                    t1 = System.currentTimeMillis();
                }
                else {
                    totalC++;
                }

                if (--loops < 1) break;
            }

            t2 = System.currentTimeMillis();
            deltaT = t2 - t1; // millisec
            freqAvg = (double) totalC / deltaT * 1000;

            System.out.println("Time = " + deltaT + " msec,  Hz = " + freqAvg);

            System.out.println("Finished all loops, count = " + totalC);

//        // Create our own record
//        RecordOutputStream myRecord = new RecordOutputStream(writer.getByteOrder());
//        buffer = TestWriter.generateBuffer(200);
//        myRecord.addEvent(buffer);
//        myRecord.addEvent(buffer);
//        writer.writeRecord(myRecord);

//        writer.addTrailer(true);
//        writer.addTrailerWithIndex(true);
            writer.close();

            System.out.println("Finished writing file");


    }


public static void testStreamRecordMT(){

        // Variables to track record build rate
        double freqAvg;
        long t1, t2, deltaT,

                totalC=0;
        // Ignore the first N values found for freq in order
        // to get better avg statistics. Since the JIT compiler in java
        // takes some time to analyze & compile code, freq may initially be low.
        long ignore = 0;
        long loops  = 6;

        string fileName = "/daqfs/home/timmer/exampleFile.v6.evio";

        // Create files
        WriterMT writer1 = new WriterMT(fileName + ".1", ByteOrder::ENDIAN_LITTLE, 0, 0,
                                        1, 1, 8);
        WriterMT writer2 = new WriterMT(fileName + ".2", ByteOrder::ENDIAN_LITTLE, 0, 0,
                                        1, 2, 8);
        WriterMT writer3 = new WriterMT(fileName + ".3", ByteOrder::ENDIAN_LITTLE, 0, 0,
                                        1, 3, 8);

        byte[] buffer = generateBuffer(400);

        t1 = System.currentTimeMillis();

        while (true) {
            // random data array
            writer1.addEvent(buffer);
            writer2.addEvent(buffer);
            writer3.addEvent(buffer);

//System.out.println(""+ (20000000 - loops));
            // Ignore beginning loops to remove JIT compile time
            if (ignore-- > 0) {
                t1 = System.currentTimeMillis();
            }
            else {
                totalC++;
            }

            if (--loops < 1) break;
        }

        t2 = System.currentTimeMillis();
        deltaT = t2 - t1; // millisec
        freqAvg = (double) totalC / deltaT * 1000;

        System.out.println("Time = " + deltaT + " msec,  Hz = " + freqAvg);

        System.out.println("Finished all loops, count = " + totalC);


        writer1.addTrailer(true);
        writer1.addTrailerWithIndex(true);

        writer2.addTrailer(true);
        writer2.addTrailerWithIndex(true);

        writer3.addTrailer(true);
        writer3.addTrailerWithIndex(true);


        writer1.close();
        writer2.close();
        writer3.close();

        // Doing a diff between files shows they're identical!

        System.out.println("Finished writing files");


    }


public static void streamRecord() throws HipoException {
            RecordOutputStream stream = new RecordOutputStream();
            byte[] buffer = TestWriter.generateBuffer();
            while(true){
                //byte[] buffer = TestWriter.generateBuffer();
                boolean flag = stream.addEvent(buffer);
                if(flag==false){
                    stream.build();
                    stream.reset();
                }
            }

    }

public static void writerTest() throws IOException {
            Writer writer = new Writer("compressed_file.evio",
            ByteOrder.BIG_ENDIAN, 0, 0);
            //byte[] array = TestWriter.generateBuffer();
            for(int i = 0; i < 340000; i++){
                byte[] array = TestWriter.generateBuffer();
                writer.addEvent(array);
            }
            writer.close();
    }


public static void convertor() {
        String filename = "/Users/gavalian/Work/Software/project-1a.0.0/clas_000810.evio.324";
        try {
            EvioCompactReader  reader = new EvioCompactReader(filename);
            int nevents = reader.getEventCount();
            String userHeader = "File is written with new version=6 format";
            Writer writer = new Writer("converted_000810.evio",ByteOrder.LITTLE_ENDIAN,
                                       10000,8*1024*1024);
            writer.setCompressionType(2);

            System.out.println(" OPENED FILE EVENT COUNT = " + nevents);

            byte[] myHeader = new byte[233];
            ByteBuffer header = ByteBuffer.wrap(myHeader);
            //nevents = 560;
            for(int i = 1; i < nevents; i++){
                ByteBuffer buffer = reader.getEventBuffer(i,true);
                writer.addEvent(buffer.array());
                //String data = DataUtils.getStringArray(buffer, 10, 30);
                //System.out.println(data);
                //if(i>10) break;
                //System.out.println(" EVENT # " + i + "  size = " + buffer.array().length );
            }
            writer.close();
        } catch (EvioException ex) {
            Logger.getLogger(TestWriter.class.getName()).log(Level.SEVERE, null, ex);
        } catch (IOException ex) {
            Logger.getLogger(TestWriter.class.getName()).log(Level.SEVERE, null, ex);
        }

    }


public static void createEmptyFile() throws IOException, HipoException {
            String userHeader = "Example of creating a new header file.......?";
            System.out.println("STRING LENGTH = " + userHeader.length());
            Writer writer = new Writer();
            writer.open("example_file.evio", userHeader.getBytes());
            for(int i = 0; i < 5; i++){
                byte[] array = TestWriter.generateBuffer();
                writer.addEvent(array);
            }

            writer.close();
    }

public static void main(String[] args){

        testStreamRecordMT();

        /*Writer writer = new Writer();

        writer.open("new_header_test.evio",new byte[]{'a','b','c','d','e'});
        writer.close(); */

        //writer.createHeader(new byte[17]);

        //TestWriter.convertor();

        //TestWriter.writerTest();

        //TestWriter.streamRecord();

        //TestWriter.byteStream();

        /*
        byte[] header = TestWriter.generateBuffer(32);

        Writer writer = new Writer("compressed.evio", header,1);

        for(int i = 0 ; i < 425; i++){
            byte[] array = TestWriter.generateBuffer();
            writer.addEvent(array);
        }

        writer.close();

        Reader reader = new Reader();
        reader.open("compressed.evio");
        reader.show();
        Record record = reader.readRecord(0);
        record.show();
        */


        /*
        Record record = new Record();
        for(int i = 0; i < 6; i++){
            byte[] bytes = TestWriter.generateBuffer();
            record.addEvent(bytes);
        }

        record.show();
        record.setCompressionType(0);
        byte[] data = record.build().array();
        System.out.println(" BUILD BUFFER SIZE = " + data.length);
        TestWriter.print(data);

        Record record2 = Record.initBinary(data);

        record2.show();*/
    }





};


#endif //EVIO_6_0_READWRITETEST_H