/*
 *   Copyright (c) 2017.  Jefferson Lab (JLab). All rights reserved. Permission
 *   to use, copy, modify, and distribute  this software and its documentation for
 *   educational, research, and not-for-profit purposes, without fee and without a
 *   signed licensing agreement.
 */
package org.jlab.coda.hipo;

import com.lmax.disruptor.Sequence;

import java.nio.ByteOrder;

/**
 * This class is used in conjunction with the {@link RecordSupply} class
 * to provide a very fast supply of RecordOutputStream objects for reuse.
 * Objects of this class are used to populate the ring buffer in RecordSupply
 * and are simply a small wrapper around RecordOutputStream objects.
 * Uses the Disruptor software package.
 *
 * @author timmer
 * @version 6.0
 * @since 6.0 9/21/17
 */
public class RecordRingItem {

    /** Record object. */
    private RecordOutputStream record;

    /** Byte order of record being built. */
    private final ByteOrder order;

    /** Sequence at which this object was taken from ring by one of the "get" calls. */
    private long sequence;

    /** Sequence object allowing ring consumer to get/release this item. */
    private Sequence sequenceObj;

    /** Do we split a file after writing this record? */
    private volatile boolean splitFileAfterWrite;

    /** Do we force the record to be physically written to disk? */
    private volatile boolean forceToDisk;

    /** Processing thread may need to know if this is the last item
     *  to be processed so thread can shutdown. */
    private volatile boolean lastItem;

     //--------------------------------

    /**
     * Constructor.
     * @param order         byte order of built record byte arrays.
     * @param maxEventCount max number of events record can hold.
     *                      Value <= O means use default (1M).
     * @param maxBufferSize max number of uncompressed data bytes record can hold.
     *                      Value of < 8MB results in default of 8MB.
     * @param compressionType type of data compression to do (0=none, 1=lz4 fast, 2=lz4 best, 3=gzip).
     */
    RecordRingItem(ByteOrder order, int maxEventCount, int maxBufferSize, int compressionType) {
        this.order = order;
        record = new RecordOutputStream(order, maxEventCount, maxBufferSize, compressionType);
    }


    /** Method to reset this item each time it is retrieved from the supply. */
    public void reset() {
        record.reset();

        sequence = 0L;
        sequenceObj = null;
        
        lastItem = false;
        forceToDisk = false;
        splitFileAfterWrite = false;
    }

    /**
     * Get the contained record. Record is reset.
     * @return contained record.
     */
    public RecordOutputStream getRecord() {return record;}

    /**
     * Get the byte order used to build record.
     * @return byte order used to build record.
     */
    public ByteOrder getOrder() {return order;}

    /**
     * Get the sequence at which this object was taken from ring by one of the "get" calls.
     * @return sequence at which this object was taken from ring by one of the "get" calls.
     */
    public long getSequence() {return sequence;}

    /**
     * Get the Sequence object allowing ring consumer to get/release this item.
     * @return Sequence object allowing ring consumer to get/release this item.
     */
    public Sequence getSequenceObj() {return sequenceObj;}

    /**
     * Set the sequence of an item obtained through {@link RecordSupply#get()}.
     * @param seq sequence used to get item.
     */
    public void fromProducer(long seq) {sequence = seq;}

    /**
     * Set the sequence of an item obtained through {@link RecordSupply#getToCompress(int)}.
     * @param seq sequence used to get item.
     * @param seqObj sequence object used to get/release item.
     */
    public void fromConsumer(long seq, Sequence seqObj) {
        sequence = seq;
        sequenceObj = seqObj;
    }

    /**
     * Get whether a file writer splits the file after writing this record.
     * @return true if file writer splits the file after writing this record.
     */
    public boolean splitFileAfterWrite() {return splitFileAfterWrite;}

    /**
     * Set whether a file writer splits the file after writing this record.
     * @param split if true, file writer splits the file after writing this record,
     *              else false.
     */
    public void splitFileAfterWrite(boolean split) {splitFileAfterWrite = split;}

    /**
     * Get whether a file writer forces this record to be physically written to disk.
     * @return true if file writer forces this record to be physically written to disk.
     */
    public boolean forceToDisk() {return forceToDisk;}

    /**
     * Set whether a file writer forces this record to be physically written to disk.
     * @param force if true, file writer forces this record to be physically written
     *              to disk, else false.
     */
    public void forceToDisk(boolean force) {forceToDisk = force;}

    /**
     * Get whether this is the last item in the supply to be used.
     * @return true this is the last item in the supply to be used.
     */
    public boolean isLastItem() {return lastItem;}

    /**
     * Set whether this is the last item in the supply to be used.
     * Used in WriterMT when closing.
     * @param last if true, this is the last item in the supply to be used.
     */
    public void setLastItem(boolean last) {lastItem = last;}

}