#ifndef NS_TTF_H
#define NS_TTF_H

#include "ns_math.h"

/*
shortFrac	16-bit signed fraction
Fixed	16.16-bit signed fixed-point number
FWord	16-bit signed integer that describes a quantity in FUnits, the smallest measurable distance in em space.
uFWord	16-bit unsigned integer that describes a quantity in FUnits, the smallest measurable distance in em space.
F2Dot14	16-bit signed fixed number with the low 14 bits representing fraction.
longDateTime	The long internal format of a date in seconds since 12:00 midnight, January 1, 1904. It is represented as a signed 64-bit integer.
*/

typedef uint32_t Fixed;
typedef int16_t FWORD;
typedef uint16_t UFWORD;
typedef uint16_t F2DOT14;
typedef uint64_t LONGDATETIME;

/* Simple glyph flags. */
#define ON_CURVE_POINT 0X01
#define X_SHORT_VECTOR 0X02
#define Y_SHORT_VECTOR 0X04
#define REPEAT_FLAG 0x08
#define X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR 0x10
#define Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR 0x20
#define OVERLAP_SIMPLE 0x40

/* Composite glyph flags. */
#define ARG_1_AND_2_ARE_WORDS 0x0001
#define ARGS_ARE_XY_VALUES 0x0002
#define ROUND_XY_TO_GRID 0x0004
#define WE_HAVE_A_SCALE 0x0008
#define MORE_COMPONENTS 0x0020
#define WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define WE_HAVE_A_TWO_BY_TWO 0x0080
#define WE_HAVE_INSTRUCTIONS 0x0100
#define USE_MY_METRICS 0x0200
#define OVERLAP_COMPOUND 0x0400
#define SCALED_COMPONENT_OFFSET 0x0800
#define UNSCALED_COMPONENT_OFFSET 0x1000
#define Reserved 0xE010

/* Size of the glyph header i.e. numContours and x/y min/max. */
#define GLYPH_HEADER_SIZE (sizeof(int16_t) + sizeof(int16_t) + sizeof(int16_t) + sizeof(int16_t) + sizeof(int16_t))

#pragma pack(push, 1)
struct offset_subtable
{
    uint32_t scalerType;
    uint16_t numTables;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;
};
global int GlobalOffsetSubtableSizes[] { sizeof(uint32_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t) };

struct table_directory_entry
{
    uint32_t tag;
    uint32_t checkSum;
    uint32_t offset;
    uint32_t length;
};
global int GlobalTableDirectoryEntrySizes[] { sizeof(uint32_t), sizeof(uint32_t), sizeof(uint32_t), sizeof(uint32_t) };

struct head_table
{
    uint16_t majorVersion;
    uint16_t minorVersion;
    Fixed fontRevision;
    uint32_t checkSumAdjustment;
    uint32_t magicNumber;
    uint16_t flags;
    uint16_t unitsPerEm;
    int64_t created;
    int64_t modified;
    int16_t xMin;
    int16_t yMin;
    int16_t xMax;
    int16_t yMax;
    uint16_t macStyle;
    uint16_t lowestRecPPEM;
    int16_t fontDirectionHint;
    int16_t indexToLocFormat;
    int16_t glyphDataFormat;
};
global int GlobalHeadTableSizes[] = { sizeof(uint16_t), sizeof(uint16_t), sizeof(Fixed), sizeof(uint32_t), sizeof(uint32_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(int64_t), sizeof(int64_t), sizeof(int16_t), sizeof(int16_t), sizeof(int16_t), sizeof(int16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(int16_t), sizeof(int16_t), sizeof(int16_t), };

struct maxp_table
{
    uint32_t version;
    uint16_t numGlyphs;
    uint16_t maxPoints;
    uint16_t maxContours;
    uint16_t maxCompositePoints;
    uint16_t maxCompositeContours;
    uint16_t maxZones;
    uint16_t maxTwilightPoints;
    uint16_t maxStorage;
    uint16_t maxFunctionDefs;
    uint16_t maxInstructionDefs;
    uint16_t maxStackElements;
    uint16_t maxSizeOfInstructions;
    uint16_t maxComponentElements;
    uint16_t maxComponentDepth;
};
global int GlobalMaxpTableSizes[] = { sizeof(uint32_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t) };

struct encoding_record
{
    uint16_t platformID;
    uint16_t platformSpecificID;
    uint32_t offset;
};
global int GlobalEncodingRecordSizes[] = { sizeof(uint16_t), sizeof(uint16_t), sizeof(uint32_t), };

struct cmap_table_entry
{
    uint16_t format;
    uint16_t length;
    uint16_t language;
    uint16_t segCountX2;
    uint16_t searchRange;
    uint16_t entrySelector;
    uint16_t rangeShift;
    uint16_t firstEndCount;

    uint16_t *endCount;
    uint16_t reservedPad;
    uint16_t *startCount;
    int16_t *idDelta;
    uint16_t *idRangeOffset;
    uint16_t *glyphIdArray;

    /* Convenience. */
    uint32_t SegmentCount;
    uint32_t NumGlyphIds;
};
global int GlobalCmapTableEntrySizes[] = { sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), };

struct cmap_table
{
    uint16_t version;
    uint16_t numberSubtables;
    encoding_record FirstEncodingRecord;

    encoding_record *EncodingRecords;
    cmap_table_entry *Entries;
    cmap_table_entry FirstEntry;
};
global int GlobalCmapTableSizes[] = { sizeof(uint16_t), sizeof(uint16_t), };

struct glyph_table_entry
{
    int16_t numberOfContours;
    uint16_t xMin;
    uint16_t yMin;
    uint16_t xMax;
    uint16_t yMax;
    uint16_t FirstEndPtsOfCountours;

    uint16_t *endPtsOfCountours;
    uint16_t instructionLength;
    uint8_t *instructions;
    uint8_t *flags;
    uint8_t *xCoordinates;
    uint8_t *yCoordinates;
};
global int GlobalGlyphTableEntrySizes[] = { sizeof(int16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), sizeof(uint16_t), };
#pragma pack(pop)

struct glyph_access_table_entry
{
    uint8_t *Xs;
    uint8_t *Ys;

    /* TODO: it wouldn't be too difficult locate these without the table... */
    int NumContours;
    int NumPoints;
    uint16_t *ContourEndPoints;
    uint8_t *Flags;
    int16_t MinX;
    int16_t MinY;
    int16_t MaxX;
    int16_t MaxY;
};

struct ns_ttf
{
    ns_file File;
    offset_subtable *OffsetSubtable;
    table_directory_entry *TableDirectoryEntries;
    head_table *HeadTable;
    maxp_table *MaxpTable;
    cmap_table CmapTable;
    void *LocaTable;
    uint8_t *GlyphTable;
    glyph_access_table_entry *GlyphAccessTable;
};

enum glyph_iterator_state
{
    GlyphIteratorState_HasMore,
    GlyphIteratorState_IsLast,
    GlyphIteratorState_HasNoMore,
};

struct ttf_flag
{
    uint8_t *Ptr;
    uint8_t NumRepeats;
};

struct glyph_coord_iterator
{
    int CoordNumber;
    int NumPoints;
    int Size;
    uint8_t *Ptr;
    int Value;
    bool IsX;
    bool DoFix;
};

struct glyph_iterator
{
	ns_ttf *File;

    glyph_iterator_state State;
    uint8_t *Ptr;

	/* Simple glyphs. */
	int NumContours;

	/* Compound glyphs. */
    int NumInstructions;
    uint8_t *Instructions;
    int GlyphId;
    int XOffset;
    int YOffset;
};

struct glyph_contour_iterator;
struct glyph_contour_point_iterator
{
	glyph_contour_iterator *ContourParent;
    int PointIdx;
    ttf_flag Flag;
    glyph_coord_iterator X;
    glyph_coord_iterator Y;
    uint8_t *DebugXCoordsStart;
    uint8_t *DebugFlagsStart;
};

struct glyph_contour_iterator
{
	glyph_iterator *Parent;
	glyph_contour_point_iterator PointIterator;
	int ContourNumber;
	int NumPoints;
};

internal void *
GetTable(ns_ttf *File, const char *TagString)
{
    uint32_t Tag = ConvertStringToInt(TagString);
    void *Result = NULL;
    for (int TableIdx = 0; TableIdx < File->OffsetSubtable->numTables; TableIdx++)
    {
        table_directory_entry *TableDirectoryEntries = &File->TableDirectoryEntries[TableIdx];
        if (TableDirectoryEntries->tag == Tag)
        {
            Result = File->File.Contents + TableDirectoryEntries->offset;
            break;
        }
    }
    return Result;
}

internal void
Advance(ttf_flag *Flag)
{
    if (Flag->NumRepeats > 0)
    {
        Flag->NumRepeats--;
        if (!Flag->NumRepeats)
        {
            Flag->Ptr += 2;
        }
    }
    else
    {
        if (*Flag->Ptr & REPEAT_FLAG)
        {
            Flag->NumRepeats = *(Flag->Ptr + 1);
        }
        else
        {
            Flag->Ptr++;
        }
    }
}

internal ttf_flag
CreateFlag(uint8_t *FlagPtr)
{
    ttf_flag Result = {};
    Result.Ptr = FlagPtr;
    return Result;
}

internal int
GetNumFlagBytes(uint8_t *Flags, int NumPoints)
{
    ttf_flag Flag = { Flags, 0 };
    int NumFlags = 0;
    for (uint16_t PointIdx = 0; PointIdx < NumPoints; PointIdx++)
    {
        Advance(&Flag);
    }
    int Result = (Flag.Ptr - Flags);
    return Result;
}

internal int
GetGlyphOffset(ns_ttf *File, int GlyphId)
{
    int Result;
    if (File->HeadTable->indexToLocFormat == 0)
    {
        uint16_t *LocaTable = (uint16_t *)File->LocaTable;
        Result = LocaTable[GlyphId]*2;
    }
    else
    {
        uint32_t *LocaTable = (uint32_t *)File->LocaTable;
        Result = LocaTable[GlyphId];
    }
    return Result;
}

internal uint8_t *
GetGlyphTableEntry(ns_ttf *File, int GlyphId)
{
    uint8_t *Result = File->GlyphTable + GetGlyphOffset(File, GlyphId);
    return Result;
}

internal uint8_t *
GetEndOfGlyphTable(ns_ttf *File)
{
    int EndOfGlyphTableOffset = GetGlyphOffset(File, File->MaxpTable->numGlyphs);
    uint8_t *Result = File->GlyphTable + EndOfGlyphTableOffset;
    return Result;
}

internal int
GetGlyphId(ns_ttf *File, char Char)
{
    bool FoundRange = false;
    uint32_t RangeIdx = 0;
    for (; RangeIdx < File->CmapTable.FirstEntry.SegmentCount; RangeIdx++)
    {
        if (Char < File->CmapTable.FirstEntry.endCount[RangeIdx])
        {
            FoundRange = true;
            break;
        }
    }
    Assert(FoundRange);

    cmap_table_entry *CmapTable = &File->CmapTable.FirstEntry;
    int GlyphId;
    if (CmapTable->idRangeOffset[RangeIdx])
    {
        GlyphId = *(&CmapTable->idRangeOffset[RangeIdx] + CmapTable->idRangeOffset[RangeIdx]/2 + (Char - CmapTable->startCount[RangeIdx]));
    }
    else
    {
        GlyphId = CmapTable->idDelta[RangeIdx] + Char;
    }

    return GlyphId;
}

internal bool
CheckGlyphSimple(ns_ttf *File, int GlyphId)
{
    bool Result = File->GlyphAccessTable[GlyphId].NumContours >= 0;
    return Result;
}

internal void
Advance(glyph_coord_iterator *Coord, ttf_flag *Flag)
{
    Assert(Coord->CoordNumber <= Coord->NumPoints);
    if (Coord->CoordNumber < Coord->NumPoints)
    {
        uint8_t XYShortBit = Coord->IsX ? X_SHORT_VECTOR : Y_SHORT_VECTOR;
        uint8_t XYSameBit = Coord->IsX ? X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR : Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR;

        bool IsShortBitSet = (*Flag->Ptr & XYShortBit);
        bool IsSameBitSet = (*Flag->Ptr & XYSameBit);
        bool IsSameAsPrev = (!IsShortBitSet && IsSameBitSet);
        int Relative = 0;
        if (!IsSameAsPrev)
        {
            Coord->Ptr += Coord->Size;
            Coord->Size = IsShortBitSet ? 1 : 2;
            if (Coord->DoFix) FixBigEndian(Coord->Ptr, Coord->Size);
            if (IsShortBitSet)
            {
                int Sign = IsSameBitSet ? 1 : -1;
                Relative = Sign*(*(uint8_t *)Coord->Ptr);
            }
            else
            {
                Relative = *(int16_t *)Coord->Ptr;
            }
        }
        Coord->Value += Relative;
    }
    else
    {
        /* Like other iterators, we stop _after_ we've moved one off the end. This means for the last iteration, the 
           coordinate/flag we'd use is also one off the end, or in other words, garbage. Just dont't do it! */
        Coord->Ptr += Coord->Size;
    }
    Coord->CoordNumber++;
}

internal glyph_coord_iterator
CreateGlyphCoordIterator(uint8_t *Coordinates, ttf_flag *Flag, int NumPoints, bool IsX, bool DoFix = false)
{
    glyph_coord_iterator Result = {};
    Result.Ptr = Coordinates;
    Result.NumPoints = NumPoints;
    Result.IsX = IsX;
    Result.DoFix = DoFix;
    Advance(&Result, Flag);
    return Result;
}

internal int 
GetNumPointsInContour(ns_ttf *File, int GlyphId, int ContourIdx)
{
	uint16_t *ContourEndPoints = File->GlyphAccessTable[GlyphId].ContourEndPoints;
    int Result;
    if (ContourIdx == 0)
    {
        Result = ContourEndPoints[ContourIdx] + 1;
    }
    else
    {
        Result = ContourEndPoints[ContourIdx] - ContourEndPoints[ContourIdx - 1];
    }
    return Result;
}

internal uint8_t
GetFlag(glyph_contour_point_iterator *Point)
{
    Assert(Point->Flag.Ptr < Point->DebugXCoordsStart);
    uint8_t Result = *Point->Flag.Ptr;
    return Result;
}

internal int
GetX(glyph_contour_point_iterator *Point)
{
    int Result = Point->X.Value;
    return Result;
}

internal int
GetY(glyph_contour_point_iterator *Point)
{
    int Result = Point->Y.Value;
    return Result;
}

internal bool
HasMore(glyph_contour_point_iterator *Point)
{
    bool Result = Point->PointIdx < Point->ContourParent->NumPoints;
    return Result;
}

internal void
Advance(glyph_contour_point_iterator *Point)
{
    Advance(&Point->Flag);
    Advance(&Point->X, &Point->Flag);
    Advance(&Point->Y, &Point->Flag);
    Assert(Point->Flag.Ptr <= Point->DebugXCoordsStart);
    Point->PointIdx++;
}

internal glyph_contour_point_iterator
CreatePointIterator(ns_ttf *File, int GlyphId)
{
    glyph_contour_point_iterator Result = {};
    Result.Flag.Ptr = File->GlyphAccessTable[GlyphId].Flags;
    Result.DebugFlagsStart = Result.Flag.Ptr;
    Result.X.Ptr = File->GlyphAccessTable[GlyphId].Xs;
    Result.DebugXCoordsStart = Result.X.Ptr;
    Result.Y.Ptr = File->GlyphAccessTable[GlyphId].Ys;
    Assert(Result.X.Ptr && Result.Y.Ptr);
	int TotalNumPoints = File->GlyphAccessTable[GlyphId].NumPoints;
    Result.X = CreateGlyphCoordIterator(Result.X.Ptr, &Result.Flag, TotalNumPoints, true);
    Result.Y = CreateGlyphCoordIterator(Result.Y.Ptr, &Result.Flag, TotalNumPoints, false);
    return Result;
}

internal glyph_contour_point_iterator *
GetPointIterator(glyph_contour_iterator *Iterator)
{
	glyph_contour_point_iterator *Result = &Iterator->PointIterator;
	Result->ContourParent = Iterator;
	return Result;
}

internal bool
HasMore(glyph_contour_iterator *Iterator)
{
	bool Result;
	if (Iterator->ContourNumber == 1)
	{
		/* This is the first time we've been called. The point iterator hasn't been setup yet, so don't call it. */
		Result = true;
	}
	else
	{
		Result = Iterator->ContourNumber <= Iterator->Parent->NumContours;
	}
	return Result;
}

internal void
Advance(glyph_contour_iterator *Iterator)
{
	Iterator->NumPoints = GetNumPointsInContour(Iterator->Parent->File, Iterator->Parent->GlyphId, Iterator->ContourNumber);
	Iterator->PointIterator.PointIdx = 0;
	Iterator->ContourNumber++;
}

internal glyph_contour_iterator
GetContourIterator(glyph_iterator *GlyphIterator)
{
	glyph_contour_iterator Result = {};
	Result.Parent = GlyphIterator;
	Result.Parent->NumContours;
	Result.PointIterator = CreatePointIterator(GlyphIterator->File, GlyphIterator->GlyphId);
	Advance(&Result);
	return Result;
}

internal bool
HasMore(glyph_iterator *Iterator)
{
    bool Result = (Iterator->State != GlyphIteratorState_HasNoMore);
    return Result;
}

internal void
Advance(glyph_iterator *Iterator, bool Fix = false, bool First = false)
{
	Assert(Iterator->NumContours >= 0);
	Assert(Iterator->State != GlyphIteratorState_HasNoMore);
    if (Iterator->State == GlyphIteratorState_IsLast)
    {
        Iterator->State = GlyphIteratorState_HasNoMore;
        return;
    }

    uint16_t *Flag = (uint16_t *)Iterator->Ptr;
    if (First)
    {
        int16_t *numberOfContours = (int16_t *)Iterator->Ptr;
        if (Fix) FixBigEndian(numberOfContours);
        int16_t *xMin = numberOfContours + 1;
        if (Fix) FixBigEndian(xMin);
        int16_t *yMin = xMin + 1;
        if (Fix) FixBigEndian(yMin);
        int16_t *xMax = yMin + 1;
        if (Fix) FixBigEndian(xMax);
        int16_t *yMax = xMax + 1;
        if (Fix) FixBigEndian(yMax);
        Flag = (uint16_t *)(yMax + 1);
    }
    if (Fix) FixBigEndian(Flag);
    uint16_t *GlyphIndex = Flag + 1;
    if (Fix) FixBigEndian(GlyphIndex);
    Iterator->GlyphId = *GlyphIndex;

    uint8_t *Next;
    if (*Flag & ARG_1_AND_2_ARE_WORDS)
    {
        uint16_t *Argument1 = GlyphIndex + 1;
        if (Fix) FixBigEndian(Argument1);
        uint16_t *Argument2 = Argument1 + 1;
        if (Fix) FixBigEndian(Argument2);
        Iterator->XOffset = *Argument1;
        Iterator->YOffset = *Argument2;

        Next = (uint8_t *)(Argument2 + 1);
    }
    else
    {
        uint8_t *Argument1 = (uint8_t *)(GlyphIndex + 1);
        uint8_t *Argument2 = Argument1 + 1;
        Iterator->XOffset = *Argument1;
        Iterator->YOffset = *Argument2;

        Next = Argument2 + 1;
    }

    if (*Flag & WE_HAVE_A_SCALE)
    {
        uint16_t *Scale = (uint16_t *)Next;
        if (Fix) FixBigEndian(Scale);

        Next = (uint8_t *)(Scale + 1);
    }
    else if (*Flag & WE_HAVE_AN_X_AND_Y_SCALE)
    {
        uint16_t *XScale = (uint16_t *)Next;
        if (Fix) FixBigEndian(XScale);
        uint16_t *YScale = XScale + 1;
        if (Fix) FixBigEndian(YScale);

        Next = (uint8_t *)(YScale + 1);
    }
    else if (*Flag & WE_HAVE_A_TWO_BY_TWO)
    {
        uint16_t *XScale = (uint16_t *)Next;
        if (Fix) FixBigEndian(XScale);
        uint16_t *Scale01 = XScale + 1;
        if (Fix) FixBigEndian(Scale01);
        uint16_t *Scale10 = Scale01 + 1;
        if (Fix) FixBigEndian(Scale10);
        uint16_t *YScale = Scale10 + 1;
        if (Fix) FixBigEndian(YScale);

        Next = (uint8_t *)(YScale + 1);
    }

    bool HasMoreComponents = (*Flag & MORE_COMPONENTS);
    if (HasMoreComponents)
    {
        uint8_t *NextFlagStart = Next;
        Iterator->Ptr = NextFlagStart;
    }
    else
    {
        if (*Flag & WE_HAVE_INSTRUCTIONS)
        {
            uint16_t *NumInstructions = (uint16_t *)Next;
            if (Fix) FixBigEndian(NumInstructions);
            Iterator->NumInstructions = *NumInstructions;
            uint8_t *Instructions = (uint8_t *)(NumInstructions + 1);
            Iterator->Instructions = Instructions;
        }
        else
        {
            Iterator->NumInstructions = 0;
            Iterator->Instructions = Next;
        }
        Iterator->State = GlyphIteratorState_IsLast;
    }
}

internal glyph_iterator
GetGlyphIterator(ns_ttf *File, int GlyphId)
{
	glyph_iterator Result = {};
	Result.File = File;
    Result.Ptr = GetGlyphTableEntry(File, GlyphId);
	Result.NumContours = *(int16_t *)Result.Ptr;
	if (Result.NumContours >= 0)
	{
		Result.State = GlyphIteratorState_IsLast;
		Result.GlyphId = GlyphId;
	}
	else
	{
		/* Note: This sets GlyphId. */
		Advance(&Result);
	}
	return Result;
}

internal glyph_iterator
GetGlyphIterator(ns_ttf *File, char Ascii)
{
	int GlyphId = GetGlyphId(File, Ascii);
	glyph_iterator Result = GetGlyphIterator(File, GlyphId);
	return Result;
}

internal int 
FixCoordinates(uint8_t *Flags, uint8_t *Coordinates, int NumPoints, bool IsX, int DebugMin, int DebugMax)
{
    ttf_flag Flag = CreateFlag(Flags);
    glyph_coord_iterator Coord = CreateGlyphCoordIterator(Coordinates, &Flag, NumPoints, IsX, true);
    for (uint16_t PointIdx = 0; PointIdx < NumPoints; PointIdx++)
    {
        Assert(Coord.Value >= DebugMin && Coord.Value <= DebugMax);
        Advance(&Flag);
        Advance(&Coord, &Flag);
    }
    uint32_t CoordinatesSize = (Coord.Ptr - Coordinates);
    return CoordinatesSize;
}

internal int
GetNumPointsInGlyph(ns_ttf *TtfFile, int GlyphId)
{
    int Result = TtfFile->GlyphAccessTable[GlyphId].NumPoints;
    return Result;
}

internal int
GetNumPointsInGlyph(ns_ttf *TtfFile, char Char)
{
    int GlyphId = GetGlyphId(TtfFile, Char);
    int Result = GetNumPointsInGlyph(TtfFile, GlyphId);
    return Result;
}

internal int
GetNumContoursInGlyph(ns_ttf *TtfFile, int GlyphId)
{
    int Result = TtfFile->GlyphAccessTable[GlyphId].NumContours;
    return Result;
}

internal int
GetNumContoursInGlyph(ns_ttf *TtfFile, char Char)
{
    int GlyphId = GetGlyphId(TtfFile, Char);
    int Result = GetNumContoursInGlyph(TtfFile, GlyphId);
    return Result;
}

internal ns_ttf
LoadTtf(const char *Name)
{
    ns_ttf Result;

    Result.File = LoadFile(Name);

    /* offset subtable */
    {
        Result.OffsetSubtable = (offset_subtable *)Result.File.Contents;
        FixBigEndianStruct((uint8_t *)Result.OffsetSubtable, GlobalOffsetSubtableSizes, ArrayCount(GlobalOffsetSubtableSizes));
        Assert(Result.OffsetSubtable->numTables != 0);
    }

    /* table directory */
    {
        Result.TableDirectoryEntries = (table_directory_entry *)(Result.OffsetSubtable + 1);
        for (int I = 0; I < Result.OffsetSubtable->numTables; I++)
        {
            FixBigEndianStruct((uint8_t *)&Result.TableDirectoryEntries[I], GlobalTableDirectoryEntrySizes, ArrayCount(GlobalTableDirectoryEntrySizes));
        }
    }

    /* head table */
    {
        Result.HeadTable = (head_table *)GetTable(&Result, "head");
        FixBigEndianStruct((uint8_t *)Result.HeadTable, GlobalHeadTableSizes, ArrayCount(GlobalHeadTableSizes));
        Assert(Result.HeadTable->magicNumber == 0x5f0f3cf5);
        Assert(Result.HeadTable->unitsPerEm >= 16 && Result.HeadTable->unitsPerEm <= 16384);
        Assert(Result.HeadTable->indexToLocFormat <= 1);
    }

    /* maxp table */
    {
        Result.MaxpTable = (maxp_table *)GetTable(&Result, "maxp");
        int32_t *version = (int32_t *)Result.MaxpTable;
        FixBigEndian(version);
        Assert(*version == 0x00010000);
        FixBigEndianStruct((uint8_t *)Result.MaxpTable, GlobalMaxpTableSizes, ArrayCount(GlobalMaxpTableSizes));
    }

    /* cmap table */
    {
        cmap_table *CmapTable = (cmap_table *)GetTable(&Result, "cmap");
        Assert(CmapTable->version == 0);
        FixBigEndianStruct((uint8_t *)CmapTable, GlobalCmapTableSizes, ArrayCount(GlobalCmapTableSizes));

        encoding_record *EncodingRecords = &CmapTable->FirstEncodingRecord;
        for (int I = 0; I < CmapTable->numberSubtables; I++)
        {
            FixBigEndianStruct((uint8_t *)&EncodingRecords[I], GlobalEncodingRecordSizes, ArrayCount(GlobalEncodingRecordSizes));
        }

        Result.CmapTable = *CmapTable;
        Result.CmapTable.EncodingRecords = EncodingRecords;
        Result.CmapTable.Entries = (cmap_table_entry *)((uint8_t *)CmapTable + EncodingRecords->offset);
    }

    /* First cmap table entry. */
    {
        cmap_table_entry *CmapTableEntry = Result.CmapTable.Entries;
        FixBigEndianStruct((uint8_t *)CmapTableEntry, GlobalCmapTableEntrySizes, ArrayCount(GlobalCmapTableEntrySizes));

        Result.CmapTable.FirstEntry = *CmapTableEntry;
        Result.CmapTable.FirstEntry.SegmentCount = Result.CmapTable.FirstEntry.segCountX2/2;
        Result.CmapTable.FirstEntry.endCount = FixBigEndianArray(&CmapTableEntry->firstEndCount, Result.CmapTable.FirstEntry.SegmentCount);
        Result.CmapTable.FirstEntry.startCount = FixBigEndianArray(Result.CmapTable.FirstEntry.endCount + Result.CmapTable.FirstEntry.SegmentCount + 1, Result.CmapTable.FirstEntry.SegmentCount);
        Result.CmapTable.FirstEntry.idDelta = FixBigEndianArray((int16_t *)(Result.CmapTable.FirstEntry.startCount + Result.CmapTable.FirstEntry.SegmentCount), Result.CmapTable.FirstEntry.SegmentCount);
        Result.CmapTable.FirstEntry.idRangeOffset = FixBigEndianArray((uint16_t *)(Result.CmapTable.FirstEntry.idDelta + Result.CmapTable.FirstEntry.SegmentCount), Result.CmapTable.FirstEntry.SegmentCount);

        Result.CmapTable.FirstEntry.NumGlyphIds = 0;
        for (uint32_t I = 0; I < Result.CmapTable.FirstEntry.SegmentCount; I++)
        {
            int RangeSize = Result.CmapTable.FirstEntry.endCount[I] - Result.CmapTable.FirstEntry.startCount[I];
            Result.CmapTable.FirstEntry.NumGlyphIds += RangeSize;
        }
        Result.CmapTable.FirstEntry.glyphIdArray = FixBigEndianArray(Result.CmapTable.FirstEntry.idRangeOffset + Result.CmapTable.FirstEntry.SegmentCount, Result.CmapTable.FirstEntry.NumGlyphIds);
    }

    /* loca table */
    {
        Result.LocaTable = GetTable(&Result, "loca");
        if (Result.HeadTable->indexToLocFormat == 0)
        {
            uint16_t *LocaTable = (uint16_t *)Result.LocaTable;
            FixBigEndianArray(LocaTable, Result.MaxpTable->numGlyphs);
        }
        else
        {
            uint32_t *LocaTable = (uint32_t *)Result.LocaTable;
            FixBigEndianArray(LocaTable, Result.MaxpTable->numGlyphs);
        }
    }

    /* glyf table */
    {
        Result.GlyphAccessTable = (glyph_access_table_entry *)MemAllocZero(sizeof(glyph_access_table_entry)*Result.MaxpTable->numGlyphs);

        Result.GlyphTable = (uint8_t *)GetTable(&Result, "glyf");
        void *EndOfLastGlyphData = 0;
        for (int GlyphIdx = 0; GlyphIdx < Result.MaxpTable->numGlyphs; GlyphIdx++)
        {
            uint8_t *GlyphTableStart = GetGlyphTableEntry(&Result, GlyphIdx);
            if (GlyphIdx > 0)
            {
                uint8_t *PrevGlyphTableStart = GetGlyphTableEntry(&Result, GlyphIdx - 1);
                if (GlyphTableStart == PrevGlyphTableStart)
                {
                    continue;
                }
            }
            Assert(GlyphTableStart >= EndOfLastGlyphData);

            int16_t *numberOfContours = FixBigEndian((int16_t *)GlyphTableStart);
            Assert(*numberOfContours <= Result.MaxpTable->maxContours);
            int16_t *xMin = FixBigEndian(numberOfContours + 1);
            int16_t *yMin = FixBigEndian(xMin + 1);
            int16_t *xMax = FixBigEndian(yMin + 1);
            int16_t *yMax = FixBigEndian(xMax + 1);
            if (*numberOfContours == 0)
            {
                CrashProgram();
            }
            else if (*numberOfContours > 0)
            {
                uint16_t *endPtsOfContours = FixBigEndianArray((uint16_t *)(yMax + 1), *numberOfContours);
                uint16_t *instructionLength = FixBigEndian(endPtsOfContours + *numberOfContours);
                uint8_t *instructions = (uint8_t *)(instructionLength + 1);
                uint8_t *flags = instructions + *instructionLength;
                int NumPoints = endPtsOfContours[*numberOfContours - 1] + 1;
                int NumFlagBytes = GetNumFlagBytes(flags, NumPoints);
                uint8_t *xCoordinates = flags + NumFlagBytes;
                int xCoordinatesSize = FixCoordinates(flags, xCoordinates, NumPoints, true, *xMin, *xMax);
                uint8_t *yCoordinates = xCoordinates + xCoordinatesSize;
                int yCoordinatesSize = FixCoordinates(flags, yCoordinates, NumPoints, false, *yMin, *yMax);
                EndOfLastGlyphData = yCoordinates + yCoordinatesSize;

                Result.GlyphAccessTable[GlyphIdx].NumPoints = NumPoints;
                Result.GlyphAccessTable[GlyphIdx].MinX = *xMin;
                Result.GlyphAccessTable[GlyphIdx].MinY = *yMin;
                Result.GlyphAccessTable[GlyphIdx].MaxX = *xMax;
                Result.GlyphAccessTable[GlyphIdx].MaxY = *yMax;
                Result.GlyphAccessTable[GlyphIdx].ContourEndPoints = endPtsOfContours;
                Result.GlyphAccessTable[GlyphIdx].Flags = flags;
                Result.GlyphAccessTable[GlyphIdx].Xs = xCoordinates;
                Result.GlyphAccessTable[GlyphIdx].Ys = yCoordinates;
            }
            else
            {
                glyph_iterator CompoundGlyphIterator = {};
                CompoundGlyphIterator.Ptr = (uint8_t *)(yMax + 1);
                Advance(&CompoundGlyphIterator, true);
                while (HasMore(&CompoundGlyphIterator))
                {
                    Advance(&CompoundGlyphIterator, true);
                }
                EndOfLastGlyphData = CompoundGlyphIterator.Instructions + CompoundGlyphIterator.NumInstructions;
            }
            Result.GlyphAccessTable[GlyphIdx].NumContours = *numberOfContours;
        }
    }

    return Result;
}

internal rect2
GetBoundingBox(ns_ttf *File, char Char)
{
    int GlyphId = GetGlyphId(File, Char);
    glyph_access_table_entry *Entry = &File->GlyphAccessTable[GlyphId];
    rect2 Result;
    Result.Min = V2(Entry->MinX, Entry->MinY);
    Result.Max = V2(Entry->MaxX, Entry->MaxY);
    return Result;
}

internal float
GetFUnitsToPixels(ns_ttf *File, int PointSize, int ScreenDpi)
{
    /* Taken from https://docs.microsoft.com/en-us/typography/opentype/otspec160/ttch01 */
    float Result = (float)PointSize*(float)ScreenDpi/(72*File->HeadTable->unitsPerEm);
    return Result;
}

internal void
Free(ns_ttf *File)
{
    Assert(File->GlyphAccessTable);
    MemFree(File->GlyphAccessTable);
}

#endif