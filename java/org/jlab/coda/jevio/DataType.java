package org.jlab.coda.jevio;


/**
 * This an enum used to convert data type numerical values to a more meaningful name.
 * For example, the data type with value 0x20 corresponds to the enum BANK.
 * Mostly this is used for printing. In this version of evio, the
 * ALSOTAGSEGMENT (0x40) value was removed from this enum because
 * the upper 2 bits of a byte containing the datatype are now used
 * to store padding data.
 * 
 * @author heddle
 * @author timmer
 *
 */
public enum DataType {

	/** Unknown data type. */
	UNKNOWN32      (0x0),
	/** Unsigned 32-bit integer. */
	UINT32         (0x1),
	/** 32-bit float. */
	FLOAT32        (0x2),
	/** String. */
	CHARSTAR8      (0x3),
	/** Signed 16-bit integer. */
	SHORT16        (0x4),
	/** Unsigned 16-bit integer. */
	USHORT16       (0x5),
	/** Signed 8-bit integer. */
	CHAR8          (0x6),
	/** Unsigned 8-bit integer. */
	UCHAR8         (0x7),
	/** 64-bit double. */
	DOUBLE64       (0x8),
	/** Signed 64-bit integer. */
	LONG64         (0x9),
	/** Unsigned 64-bit integer. */
	ULONG64        (0xa),
	/** Signed 32-bit integer. */
	INT32          (0xb),
	/** Tag segment data container. */
	TAGSEGMENT     (0xc),
	/** Segment data container. */
	ALSOSEGMENT    (0xd),
	/** Bank data container. */
    ALSOBANK       (0xe),
	/** Composite data type. */
    COMPOSITE      (0xf),
	/** Bank data container. */
	BANK           (0x10),
	/** Segment data container. */
	SEGMENT        (0x20),

    //  Removed ALSOTAGSEGMENT (0x40) since it was never
    //  used and we now need that to store padding data.
    //	ALSOTAGSEGMENT (0x40),

    // These 2 types are only used when dealing with COMPOSITE data.
    // They are never transported independently and are stored in integers.

	/** Hollerit data type used in Composite data. */
    HOLLERIT       (0x21),
	/** N (32-bit integer) data type used in Composite data. */
	NVALUE         (0x22),
	/** n (16-bit integer) data type used in Composite data. */
	nVALUE         (0x23),
	/** m (8-bit integer) data type used in Composite data. */
	mVALUE         (0x24);


    /** Each name is associated with a specific evio integer value. */
    private int value;


    /** Fast way to convert integer values into DataType objects. */
    private static DataType[] intToType;


    // Fill array after all enum objects created
    static {
        intToType = new DataType[0x24 + 1];
        for (DataType type : values()) {
            intToType[type.value] = type;
        }
    }


	/**
	 * Obtain the enum from the value.
	 *
	 * @param val the value to match.
	 * @return the matching enum, or <code>null</code>.
	 */
    public static DataType getDataType(int val) {
        if (val > 0x24 || val < 0) return null;
        return intToType[val];
    }

    /**
     * Obtain the name from the value.
     *
     * @param val the value to match.
     * @return the name, or "UNKNOWN".
     */
    public static String getName(int val) {
        if (val > 0x24 || val < 0) return "UNKNOWN";
        DataType type = getDataType(val);
        if (type == null) return "UNKNOWN";
        return type.name();
    }

	/**
	 * Convenience routine to see if the given integer arg represents a data type which
	 * is a structure (a container).
	 * @param dataType data type to examine.
	 * @return <code>true</code> if the data type corresponds to one of the structure
	 * types: BANK, SEGMENT, or TAGSEGMENT.
	 */
	static public boolean isStructure(int dataType) {
		return  dataType == BANK.value    || dataType == ALSOBANK.value    ||
				dataType == SEGMENT.value || dataType == ALSOSEGMENT.value ||
				dataType == TAGSEGMENT.value;
	}

	/**
	 * Convenience routine to see if the given integer arg represents a BANK.
	 * @param dataType data type to examine.
	 * @return <code>true</code> if the data type corresponds to a BANK.
	 */
	static public boolean isBank(int dataType) {
		return BANK.value == dataType || ALSOBANK.value == dataType;
	}

	/**
	 * Convenience routine to see if the given integer arg represents a SEGMENT.
	 * @param dataType data type to examine.
	 * @return <code>true</code> if the data type corresponds to a SEGMENT.
	 */
	static public boolean isSegment(int dataType) {
		return SEGMENT.value == dataType || ALSOSEGMENT.value == dataType;
	}

	/**
	 * Convenience routine to see if the given integer arg represents a TAGSEGMENT.
	 * @param dataType data type to examine.
	 * @return <code>true</code> if the data type corresponds to a TAGSEGMENT.
	 */
	static public boolean isTagSegment(int dataType) {
		return TAGSEGMENT.value == dataType;
	}


	/**
	 * Constructor.
	 * @param value numerical value of the data type.
	 */
	private DataType(int value) {
		this.value = value;
	}


	/**
	 * Get the enum's value.
	 * @return the value, e.g., 0xe for a BANK
	 */
	public int getValue() {
		return value;
	}


	/**
	 * Return a string which is usually the same as the name of the
	 * enumerated value, except in the cases of ALSOSEGMENT and
	 * ALSOBANK which return SEGMENT and BANK respectively.
	 *
	 * @return name of the enumerated type
	 */
	public String toString() {
		if      (this == ALSOBANK) return "BANK";
		else if (this == ALSOSEGMENT) return "SEGMENT";
		return super.toString();
	}


	/**
	 * Convenience routine to see if "this" data type is a structure (a container.)
	 * @return <code>true</code> if the data type corresponds to one of the structure
	 * types: BANK, SEGMENT, or TAGSEGMENT.
	 */
	public boolean isStructure() {
		switch (this) {
			case BANK:
			case SEGMENT:
			case TAGSEGMENT:
			case ALSOBANK:
			case ALSOSEGMENT:
//		case ALSOTAGSEGMENT:
				return true;
			default:
				return false;
		}
	}

	/**
	 * Convenience routine to see if "this" data type is a bank structure.
	 * @return <code>true</code> if this data type corresponds to a bank structure.
	 */
	public boolean isBank() {return (this == BANK || this == ALSOBANK);}
	
	/**
	 * Convenience routine to see if "this" data type is an integer of some kind -
	 * either 8, 16, 32, or 64 bits worth.
	 * @return <code>true</code> if the data type corresponds to an integer type
	 */
	public boolean isInteger() {
		switch (this) {
			case UCHAR8:
			case CHAR8:
			case USHORT16:
			case SHORT16:
			case UINT32:
			case INT32:
			case ULONG64:
			case LONG64:
				return true;
			default:
				return false;
		}
	}



}
