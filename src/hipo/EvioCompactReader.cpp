//
// Created by Carl Timmer on 6/18/20.
//

#include "EvioCompactReader.h"



namespace evio {


    /**
     * Constructor for reading an event file.
     *
     * @param path the full path to the file that contains events.
     *             For writing event files, use an <code>EventWriter</code> object.
     * @see org.jlab.coda.jevio.EventWriter
     * @throws java.io.IOException   if read failure
     * @throws org.jlab.coda.jevio.EvioException if file arg is null
     */
    EvioCompactReader::EvioCompactReader(string const & path, bool synced) {

        if (path.empty()) {
            throw EvioException("path is empty");
        }

        /** Object for reading file. */
        ifstream inStreamRandom;
        inStreamRandom.open(path, std::ios::binary);

        initialPosition = 0;

        // Create buffer of size 32 bytes
        size_t bytesToRead = 32;
        ByteBuffer headerBuffer(bytesToRead);
        auto headerBytes = headerBuffer.array();

        // Read first 32 bytes of file header
        inStreamRandom.read(reinterpret_cast<char *>(headerBytes), bytesToRead);

        // Parse file header to find the file's endianness & evio version #
        evioVersion = Util::findEvioVersion(headerBuffer, initialPosition);
        byteOrder = headerBuffer.order();

        // This object is no longer needed
        inStreamRandom.close();

        if (evioVersion < 5) {
            reader = std::make_shared<EvioCompactReaderV4>(path, synced);
        }
        else if (evioVersion == 6) {
            reader = std::make_shared<EvioCompactReaderV6>(path, synced);
        }
        else {
            throw  EvioException("unsupported evio version (" + std::to_string(evioVersion) + ")");
        }
    }


    /**
     * Constructor for reading a buffer with option of removing synchronization
     * for much greater speed.
     * @param bb the buffer that contains events.
     * @param synced     if true, methods are synchronized for thread safety, else false.
     * @see EventWriter
     * @throws BufferUnderflowException if not enough buffer data;
     * @throws EvioException if buffer arg is null;
     *                       failure to parse first block header;
     *                       unsupported evio version.
     */
    EvioCompactReader::EvioCompactReader(std::shared_ptr<ByteBuffer> & bb, bool synced) {

        byteBuffer = bb;
        initialPosition = byteBuffer->position();

        // Parse file header to find the buffer's endianness & evio version #
        evioVersion = Util::findEvioVersion(*(byteBuffer.get()), initialPosition);
        byteOrder = byteBuffer->order();

        if (evioVersion < 4)  {
            throw EvioException("unsupported evio version (" + std::to_string(evioVersion) + "), only 4+");
        }

        if (evioVersion == 4) {
            reader = std::make_shared<EvioCompactReaderV4>(byteBuffer, synced);
        }
        else if (evioVersion == 6) {
            reader = std::make_shared<EvioCompactReaderV6>(byteBuffer, synced);
        }
        else {
            throw EvioException("unsupported evio version (" + std::to_string(evioVersion) + ")");
        }
    }


    /**
     * Constructor for reading a buffer with option of removing synchronization
     * for much greater speed.
     * @param byteBuffer the buffer that contains events.
     * @param synced     if true, methods are synchronized for thread safety, else false.
     * @see EventWriter
     * @throws BufferUnderflowException if not enough buffer data;
     * @throws EvioException if buffer arg is null;
     *                       failure to parse first block header;
     *                       unsupported evio version.
     */
    EvioCompactReader::EvioCompactReader(std::shared_ptr<ByteBuffer> & bb, EvioNodeSource & pool, bool synced) {

        byteBuffer = bb;
        initialPosition = byteBuffer->position();

        // Parse file header to find the buffer's endianness & evio version #
        evioVersion = Util::findEvioVersion(*(byteBuffer.get()), initialPosition);
        byteOrder = byteBuffer->order();

        if (evioVersion < 4)  {
            throw EvioException("unsupported evio version (" + std::to_string(evioVersion) + "), only 4+");
        }

        if (evioVersion == 4) {
            reader = std::make_shared<EvioCompactReaderV4>(byteBuffer, pool, synced);
        }
        else if (evioVersion == 6) {
            reader = std::make_shared<EvioCompactReaderV6>(byteBuffer, pool, synced);
        }
        else {
            throw EvioException("unsupported evio version (" + std::to_string(evioVersion) + ")");
        }
    }

    /** {@inheritDoc} */
    bool EvioCompactReader::isFile() {return reader->isFile();}

    /** {@inheritDoc} */
    bool EvioCompactReader::isCompressed() {return reader->isCompressed();}

    /** {@inheritDoc} */
    void EvioCompactReader::setBuffer(std::shared_ptr<ByteBuffer> & buf) {reader->setBuffer(buf);}

    /** {@inheritDoc} */
    void EvioCompactReader::setBuffer(std::shared_ptr<ByteBuffer> & buf, EvioNodeSource & pool) {
        reader->setBuffer(buf, pool);
    }

    /** {@inheritDoc} */
    std::shared_ptr<ByteBuffer> EvioCompactReader::setCompressedBuffer(std::shared_ptr<ByteBuffer> & buf, EvioNodeSource & pool) {
        return reader->setCompressedBuffer(buf, pool);
    }

    /** {@inheritDoc} */
    /*synchronized*/bool EvioCompactReader::isClosed() {return reader->isClosed();}

    /** {@inheritDoc} */
    ByteOrder EvioCompactReader::getByteOrder() {return reader->getByteOrder();}

    /** {@inheritDoc} */
    uint32_t EvioCompactReader::getEvioVersion() {return evioVersion;}

    /** {@inheritDoc} */
    string EvioCompactReader::getPath() {return reader->getPath();}

    /** {@inheritDoc} */
    ByteOrder EvioCompactReader::getFileByteOrder() {return reader->getFileByteOrder();}

    /** {@inheritDoc} */
    /*synchronized*/string EvioCompactReader::getDictionaryXML() {
        return reader->getDictionaryXML();
    }

    /** {@inheritDoc} */
    /*synchronized*/EvioXMLDictionary EvioCompactReader::getDictionary() {
        return reader->getDictionary();
    }

    /** {@inheritDoc} */
    bool EvioCompactReader::hasDictionary() {return reader->hasDictionary();}

    //    /** {@inheritDoc} */
    //    MappedByteBuffer EvioCompactReader::getMappedByteBuffer() {return reader->getMappedByteBuffer();}

    /** {@inheritDoc} */
    std::shared_ptr<ByteBuffer> EvioCompactReader::getByteBuffer() {return reader->getByteBuffer();}

    /** {@inheritDoc} */
    size_t EvioCompactReader::fileSize() {return reader->fileSize();}


    /** {@inheritDoc} */
    EvioNode EvioCompactReader::getEvent(size_t eventNumber) {
        return reader->getEvent(eventNumber);
    }

    /** {@inheritDoc} */
    EvioNode EvioCompactReader::getScannedEvent(size_t eventNumber) {
        return reader->getScannedEvent(eventNumber);
    }

    /** {@inheritDoc} */
    EvioNode EvioCompactReader::getScannedEvent(size_t evNumber, EvioNodeSource & nodeSource) {
        return reader->getScannedEvent(evNumber, nodeSource);
    }

    /** {@inheritDoc} */
    std::shared_ptr<IBlockHeader> EvioCompactReader::getFirstBlockHeader() {
        return reader->getFirstBlockHeader();
    }

    /** {@inheritDoc} */
    /*synchronized*/void EvioCompactReader::searchEvent(size_t evNumber, uint16_t tag, uint8_t num,
                                                        std::vector<EvioNode> & vec) {
        ///*synchronized*/List<EvioNode> searchEvent(size_t evNumber, uint16_t tag, uint8_t num) {
        return reader->searchEvent(evNumber, tag, num, vec);
    }

    /** {@inheritDoc} */
    /*synchronized*/void EvioCompactReader::searchEvent(size_t eventNumber, string const & dictName,
                                     EvioXMLDictionary & dictionary, std::vector<EvioNode> & vec) {
        ///*synchronized*/List<EvioNode> searchEvent(int eventNumber, string dictName,
        //                                           EvioXMLDictionary & dictionary) {

        return reader->searchEvent(eventNumber, dictName, dictionary, vec);
    }

    /** {@inheritDoc} */
    /*synchronized*/ByteBuffer EvioCompactReader::removeEvent(size_t eventNumber) {
        return reader->removeEvent(eventNumber);
    }

    /** {@inheritDoc} */
    /*synchronized*/ByteBuffer EvioCompactReader::removeStructure(EvioNode & removeNode) {
        return reader->removeStructure(removeNode);
    }

    /** {@inheritDoc} */
    /*synchronized*/ByteBuffer EvioCompactReader::addStructure(size_t eventNumber, ByteBuffer & addBuffer) {
        return reader->addStructure(eventNumber, addBuffer);
    }

    /** {@inheritDoc} */
    ByteBuffer EvioCompactReader::getData(EvioNode & node) {
        return reader->getData(node);
    }

    /** {@inheritDoc} */
    /*synchronized*/ByteBuffer EvioCompactReader::getData(EvioNode & node, bool copy) {
        return reader->getData(node, copy);
    }

    /** {@inheritDoc} */
    ByteBuffer EvioCompactReader::getEventBuffer(size_t eventNumber) {
        return reader->getEventBuffer(eventNumber);
    }

    /** {@inheritDoc} */
    /*synchronized*/ByteBuffer EvioCompactReader::getEventBuffer(size_t eventNumber, bool copy) {
        return reader->getEventBuffer(eventNumber, copy);
    }

    /** {@inheritDoc} */
    ByteBuffer EvioCompactReader::getStructureBuffer(EvioNode & node) {
        return reader->getStructureBuffer(node);
    }

    /** {@inheritDoc} */
    /*synchronized*/ByteBuffer EvioCompactReader::getStructureBuffer(EvioNode & node, bool copy) {
        return reader->getStructureBuffer(node, copy);
    }


    /** {@inheritDoc} */
    /*synchronized*/void EvioCompactReader::close() {reader->close();}

    /** {@inheritDoc} */
    uint32_t EvioCompactReader::getEventCount() {return reader->getEventCount();}

    /** {@inheritDoc} */
    uint32_t EvioCompactReader::getBlockCount() {return reader->getBlockCount();}

    /** {@inheritDoc} */
    void EvioCompactReader::toFile(string fileName) {
        reader->toFile(fileName);
    }


}