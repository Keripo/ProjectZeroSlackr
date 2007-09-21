/******************************************************************************
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2000 Intel Corporation. All Rights Reserved.
//
//  VSS:
//	$Workfile: mp3decoder.c $
//      $Revision: 1.1 $
//      $Date: 2004/11/06 18:37:56 $
//      $Archive:   $
//
//  Description:
//      IPP MP3 Decoder 
//	Built using the IPP MP3 decoder primitives and binary libraries
//	Decodes bitstreams generated by encoders that are compliant
//	with the following so-called "MP3" audio standards:
//		1) ISO/IEC 11172-3, "MPEG-1" audio, Layer 3
//		2) ISO IS 13818-3, "MPEG-2", audio, Layer 3, "LSF" modes**
//			** 1 and 2-channel streams only 
//
******************************************************************************/

/************
Header files
*************/

/*
 * Standard IPP headers 
 */
#include <ippdefs.h>
#include <ippAC.h>
	/* Note: use ippaud.h with IPP 1.0 */

/*
 * Definitions and prototypes for 2-function MP3 API 
 */
#include "mp3decoder.h"

/************************
 MP3 Bit Stream Constants
*************************/
#define	SYNC_WORD	(long) 0xfff	/* MP3 bit stream sync word */
#define	SYNC_WORD_LEN	12		/* MP3 sync word length */
#define	SYNC_WORD_MASK	(long) 0xfff	/* MP3 sync word mask */
#define BITS_PER_BYTE	8		/* Bits per byte in the stream */
#define	SAMPLE_RATES	3		/* Number of possible sample rates */
#define	BIT_RATES	15		/* Number of possible bit rates */
#define	PQMF_BANDS	32	/* Number of channels in the synthesis PQMF bank */
#define	IMDCT_BLK_LEN	18	/* Long-block IMDCT length (or 3x short block IMDCT length) */
#define	MPEG1_ST_SIDE_INFO_BYTES	32	/* MPEG-1 side information byte count for 2-channel frames */
#define	MPEG1_MONO_SIDE_INFO_BYTES	17	/* MPEG-1 side information byte count for 1-channel frames */
#define	MPEG2_LSF_ST_SIDE_INFO_BYTES	17	/* MPEG-2 LSF side information byte count for 2-channel frames */
#define	MPEG2_LSF_MONO_SIDE_INFO_BYTES	9	/* MPEG-2 LSF side information byte count for 1-channel frames */

/************************
 General purpose macros 
*************************/
#define MIN(A,B)	((A)<(B)?(A):(B))

/************************
 Prototypes 
*************************/
int SeekMP3Sync(MP3BitStream *);
int GetStreamBytes(MP3BitStream *, Ipp8u *, int);
int StreamBytesRemaining(MP3BitStream *);

/************************
 Initialized Data 
*************************/

/*
 * Pre-computed number of main data "slots" (bytes) based on all possible
 * sample rate/bit rate permutations The expression for computing nSlots
 * from the sample and bit rates directly is given in ISO/IEC 11172-3
 * Adjustments must also be made for the header bytes (4), side
 * information bytes (17 or 32), the padding byte (1), and the CRC word
 * (2 bytes).  The computation is done at run time. The computation is
 * adjusted slightly for MPEG-2 LSF.  The number of side information bytes 
 * is either 9 or 17 instead of 17 or 32.  The table values are also
 * modified to reflect LSF sample rates. 
 */
int MPEG1_nSlotTable[SAMPLE_RATES][BIT_RATES] = {
	{0, 104, 130, 156, 182, 208, 261, 313, 365, 417, 522, 626, 731, 835, 1044},
	{0, 96, 120, 144, 168, 192, 240, 288, 336, 384, 480, 576, 672, 768, 960},
	{0, 144, 180, 216, 252, 288, 360, 432, 504, 576, 720, 864, 1008, 1152, 1440}
};

int MPEG2_LSF_nSlotTable[SAMPLE_RATES][BIT_RATES] = {
	{0, 26, 52, 78, 104, 130, 156, 182, 208, 261, 313, 365, 417, 470, 522},
	{0, 24, 48, 72, 96, 120, 144, 168, 192, 240, 288, 336, 384, 432, 480},
	{0, 36, 72, 108, 144, 180, 216, 252, 288, 360, 432, 504, 576, 648, 720}
};

/*******************************************************************************
// Name: DecodeMP3Frame
//
// Description:	Decodes a single MP3 frame.  Uses IPPs as functional building blocks.
//	 Compatible with streams generated by the following encoder types:
//	1) ISO/IEC 11172-3, "MPEG-1" audio, Layer 3
//	2) IS 13818-3, "MPEG-2" audio, Layer 3, LSF extensions **
//		** 1, 2-channel modes only
//
// Input Arguments:  bs	- Pointer to the mp3 input bit stream structure
//		 s	- Pointer to the mp3 decoder state
//
// Output Arguments: pcmaudio - Pointer to the PCM audio output buffer
//
// Returns:		 None.
//
*****************************************************************************/
int
DecodeMP3Frame(MP3BitStream * bs, Ipp16s * pcmaudio, MP3DecoderState * s)
{
	/***************
	 Decoder Locals  
	 ***************/
	int MainDataBegin;	/* Current start of main_data buffer */
	int NextMainDataBit = 0;	/* Index of the next * main_data bit to be decoded */
	int MainDataOK = 0;	/* main_data buffer status flag; * 0=buffer underrun; 1=buffer OK */
	int nSlots;		/* Number of input stream "slots" (bytes) * to read, then 
				 * append to main_data */
	int gr, ch;		/* Granule and channel loop indices */
	int i, j, k;		/* General purpose, as well as PQMF synthesis iteration indices */
	int offset;		/* PQMF synthesis iteration index */
	int pcmblock;		/* PQMF synthesis block index into PCM output buffer */
	int channels;		/* Number of channels = 1 or 2 */
	int SideInfoBytes;	/* Number of bytes occupied by side information */
	int RemainingSideInfoBytes;	/* Number of side information bytes remaining in the undecoded stream */
	int HuffmanLength;	/* Number of bytes in the block of Huffman code words */
	Ipp8u *FirstSclFacByte;	/* Pointer to first byte in scalefactor data stream */
	int FirstSclFacBit;	/* Index of first scalefactor bit */
	int SclFacLength;	/* Scalefactor data stream length, in terms of bits */
	int PrivateBits;	/* Private bits returned by the side information
				 * unpacking primitive */
	unsigned int StreamStart;	/* Entry-point ring buffer index, i.e., first byte to be decoded */
	int Granules;		/* Number of granules in the current frame, determined by mode */

	/**********************************************************************
	 State Variables - temporary pointers into the decoder state variables 
	 **********************************************************************/
	Ipp8u *MainDataPtr = (Ipp8u *) s->MainDataBuf;	/* Modifiable main_data pointer (scalefactors, Huffman symbols) */
	Ipp8u *MainDataBuf = s->MainDataBuf;	/* Reference main_data pointer */
	int *MainDataEnd = &(s->MainDataEnd);	/* Index of the last filled main_data position */
	IppMP3FrameHeader *FrameHdr = &(s->FrameHdr);	/* Frame header, i.e., bit rate, sample rate, modes, etc. */
	Ipp8u *HdrBuf = s->HdrBuf;	/* Reference pointer to the header decoding buffer */
	Ipp8u *pHdrBuf = s->HdrBuf;	/* Modifiable pointer to header decoding buffer */
	Ipp8s *ScaleFactor = s->ScaleFactor;	/* Scale factors */
	int *Scfsi = s->Scfsi;	/* Scalefactor select information */
	Ipp32s *IsXr = s->IsXr;	/* Huffman symbols, requantizer outputs */
	int *NonZeroBound = s->NonZeroBound;	/* Non-zero bound */
	Ipp32s *RequantBuf = s->RequantBuf;	/* Requantization workspace */
	Ipp32s *Xs = s->Xs;	/* IMDCT outputs */
	Ipp32s *OverlapAddBuf = s->OverlapAddBuf;	/* IMDCT overlap-add buffer */
	int *PreviousIMDCT = s->PreviousIMDCT;	/* Number of IMDCTs used for previous
						 * granules, channels (mode) */
	Ipp32s *PQMF_V_Buf = s->PQMF_V_Buf;	/* Synthesis PQMF "V" buffer */
	int *PQMF_V_Indx = s->PQMF_V_Indx;	/* Synthesis PQMF "V" buffer index */
	IppMP3SideInfo *SideInfoPtr = (IppMP3SideInfo *) s->SideInfo;	/* Side information for the current frame */

	/*********************
	Main MP3 Decoder Loop
	**********************/

	/*
	 * Trap empty ring buffer (buffer underrun) 
	 */
	if (bs->Head == bs->Tail)
		return (MP3_BUFFER_UNDERRUN);

	/*
	 * Track ring buffer start in case of later occurring buffer underrun; 
	 * allows buffer rewind 
	 */
	StreamStart = bs->Head;

	/*
	 * Locate sync word corresponding to the start of a new frame 
	 */
	if (!SeekMP3Sync(bs))
		return (MP3_SYNC_NOT_FOUND);

	/***********************************************************************
     IPP MP3 decoder primitive #1: Decode frame header that follows the sync word 
	***********************************************************************/

	/*
	 * Copy next 6 bytes from the ring buffer = 32 bits for header + 16
	 * bits for CRC 
	 */
	/*
	 * Trap ring buffer underrun 
	 */
	if ((GetStreamBytes(bs, &(HdrBuf[0]), 6)) < 6) {
		bs->Head = StreamStart;
		return (MP3_BUFFER_UNDERRUN);
	}
	ippsUnpackFrameHeader_MP3(&pHdrBuf, FrameHdr);

	/*
	 * Trap invalid frame headers 
	 */
	if ((FrameHdr->layer != 1) ||	/* Verify Layer 3 */
	    (FrameHdr->bitRate < 0) || (FrameHdr->bitRate >= 15) ||	/* Valid bit rate */
	    (FrameHdr->samplingFreq < 0) || (FrameHdr->samplingFreq > 2))	/* Valid sampling frequency */
		return (MP3_FRAME_HEADER_INVALID);

	/*
	 * Extract the number of channels associated with the current stream 
	 */
	/*
	 * Report to application via redundant state variable 
	 */
	channels = ((FrameHdr->mode) == MONO) ? 1 : 2;
	s->Channels = channels;

	/*
	 * Given header, configure decoder for Layer 3 of either ISO/IEC
	 * 11172-3 MPEG-1, or ISO IS13818-3 MPEG-2 LSF, 
	 */
	/*
	 * including side info bytes, number of granules, and main_data slot
	 * computation table 
	 */
	switch (FrameHdr->id) {
	case MPEG_1:
		SideInfoBytes =
			(channels == 2) ? MPEG1_ST_SIDE_INFO_BYTES : MPEG1_MONO_SIDE_INFO_BYTES;
		Granules = 2;
		break;
	case MPEG_2_LSF:
		SideInfoBytes =
			(channels == 2) ? MPEG2_LSF_ST_SIDE_INFO_BYTES : MPEG2_LSF_MONO_SIDE_INFO_BYTES;
		Granules = 1;
		break;
	}

	/*
	 * Given header, compute PCM buffer length, in samples 
	 */
	s->pcmLen = MAX_CHAN * Granules * IPP_MP3_GRANULE_LEN;

	/*
	 * If no CRC word, first two side info bytes have already been loaded
	 * into the header decoding buffer 
	 */
	if (FrameHdr->protectionBit)
		RemainingSideInfoBytes = SideInfoBytes - 2;
	else
		RemainingSideInfoBytes = SideInfoBytes;

	/************************************************************
	 IPP MP3 decoder primitive #2: Decode frame side information 
	 *************************************************************/

	/*
	 * Copy raw bits associated with the side information into a decoding
	 * buffer; trap ring buffer underrun 
	 */
	if ((GetStreamBytes(bs, &(HdrBuf[6]), RemainingSideInfoBytes)) < RemainingSideInfoBytes) {
		bs->Head = StreamStart;
		return (MP3_BUFFER_UNDERRUN);
	}
	ippsUnpackSideInfo_MP3(&pHdrBuf, SideInfoPtr, &MainDataBegin,
			       &PrivateBits, Scfsi, FrameHdr);

	/************************************************************
	 Main Data Decoding Buffer Maintainence 
	 *************************************************************/

	/*
	 * Determine the number of main data bytes (slots) to be copied from
	 * the current frame i.e., between the current and subsequent sync
	 * words Adjust for side information (17 or 32 bytes), as well as
	 * for header (20) + sync bits (12) (=4 bytes) 
	 */
	if (FrameHdr->id == MPEG_1)
		nSlots = MPEG1_nSlotTable[FrameHdr->samplingFreq][FrameHdr->bitRate] - SideInfoBytes - 4;
	else
		nSlots = MPEG2_LSF_nSlotTable[FrameHdr->samplingFreq][FrameHdr->bitRate] - SideInfoBytes - 4;

	/*
	 * Adjust nSlots for padding and CRC word 
	 */
	if (FrameHdr->paddingBit)
		nSlots++;
	if (!(FrameHdr->protectionBit))
		nSlots -= 2;

	/*
	 * Trap ring buffer underrun to avoid overwrite of main data on
	 * reentry with sufficient main data 
	 */
	if (StreamBytesRemaining(bs) < nSlots) {
		bs->Head = StreamStart;
		return (MP3_BUFFER_UNDERRUN);
	}

	/*
	 * Verify that sufficient main_data was extracted from the previous
	 * sync interval 
	 */
	MainDataOK = (*MainDataEnd - MainDataBegin >= 0);

	/*
	 * If so, copy the main data left over from previous sync interval to
	 * start of main_data buffer Note that MainDataBegin is the negative
	 * offset pointer (side info) described in ISO/IEC 11172-3 
	 */
	if (MainDataOK) {
		for (i = 0, j = *MainDataEnd - MainDataBegin; i < MainDataBegin; i++)
			MainDataBuf[i] = MainDataBuf[i + j];
		*MainDataEnd = MainDataBegin;
	}

	/*
	 * Load nslots of main data into the main data buffer 
	 */
	GetStreamBytes(bs, &(MainDataBuf[*MainDataEnd]), nSlots);
	*MainDataEnd = *MainDataEnd + nSlots;

	/*
	 * Decode the frame granules and channels, starting with scalefactors
	 * and Huffman symbols 
	 */
	if (MainDataOK) {
		/*
		 * Granule loop: Decode 1 or 2 channels for each 576-sample
		 * granule 
		 */
		for (gr = 0; gr < Granules; gr++) {
			/*
			 * Channel loop, part A: Decode scalefactors and Huffman
			 * symbols 
			 */
			for (ch = 0; ch < channels; ch++) {
				/*
				 * Point to the correct side information, given the number 
				 * of channels; the table of side information structures
				 * returned by ippsUnpackSideInfo_MP3 is organized as
				 * follows:
				 * 
				 * index one channel two channels
				 * ------------------------------------- 0 0 gr0 gr0/ch0 0 
				 * 1 gr1 gr0/ch1 1 0 empty gr1/ch0 1 1 empty gr1/ch1 
				 */
				if (channels == 2)
					SideInfoPtr = &((s->SideInfo)[gr][ch]);
				else
					SideInfoPtr = &((s->SideInfo)[0][gr]);

				/*
				 * Unpack scalefactors and measure legnth of scalefactor
				 * data block in the bitstream 
				 */
				FirstSclFacByte = MainDataPtr;
				FirstSclFacBit = NextMainDataBit;

				/***********************************************
				IPP MP3 decoder primitive #3: Unpack scalefactors for one channel of one granule 
				***********************************************/
				ippsUnpackScaleFactors_MP3_1u8s(&MainDataPtr,
								&NextMainDataBit,
								&(ScaleFactor[ch * IPP_MP3_SF_BUF_LEN]),
								SideInfoPtr,
								&(Scfsi[ch * 4]),
								FrameHdr, gr, ch);

				/***********************************************
				IPP MP3 decoder primitive #4: Decode Huffman symbols for one channel of one granule 
				***********************************************/
				/*
				 * Measure length of scalefactor block, in bits 
				 */
				SclFacLength =
					8 * (MainDataPtr - FirstSclFacByte) -
					FirstSclFacBit + NextMainDataBit;

				/*
				 * Compute Huffman data length given scalefactor block
				 * length and part23len from side info 
				 */
				HuffmanLength = SideInfoPtr->part23Len - SclFacLength;

				/*
				 * Invoke the Huffman unpacking primitive 
				 */
				ippsHuffmanDecode_MP3_1u32s(&MainDataPtr,
							    &NextMainDataBit,
							    &(IsXr[ch * IPP_MP3_GRANULE_LEN]),
							    &(NonZeroBound[ch]),
							    SideInfoPtr, FrameHdr,
							    HuffmanLength);
			}	/* end of the channel loop, part A */

			/*******************************************************
			IPP MP3 decoder primitive #5: Requantize Huffman symbols for all channels of current granule 
			*******************************************************/
			if (channels == 2)
				SideInfoPtr = &((s->SideInfo)[gr][0]);
			else
				SideInfoPtr = &((s->SideInfo)[0][gr]);
			ippsReQuantize_MP3_32s_I(IsXr, NonZeroBound, ScaleFactor, SideInfoPtr, FrameHdr, RequantBuf);

			/*
			 * Channel loop, part B: apply hybrid synthesis filter bank on 
			 * both channels of current granule 
			 */
			for (ch = 0; ch < channels; ch++) {
				/*
				 * Select the correct side information, given the number
				 * of channels (see explanation above) 
				 */
				if (channels == 2)
					SideInfoPtr = &((s->SideInfo)[gr][ch]);
				else
					SideInfoPtr = &((s->SideInfo)[0][gr]);

				/***********************************************
				 IPP MP3 decoder primitive #6: Apply IMDCT (first stage of the hybrid filter bank) 
				 **********************************************/
				ippsMDCTInv_MP3_32s(&(IsXr[ch * IPP_MP3_GRANULE_LEN]),
						    &(Xs[ch * IPP_MP3_GRANULE_LEN]),
						    &(OverlapAddBuf
						      [ch * IPP_MP3_GRANULE_LEN]),
						    NonZeroBound[ch],
						    &(PreviousIMDCT[ch]),
						    SideInfoPtr->blockType,
						    SideInfoPtr->mixedBlock);

				/***********************************************
				 IPP MP3 decoder primitive #7: Apply synthesis PQMF (second stage of the hybrid filter bank) 
				 **********************************************/
				/*
				 * PQMF synthesis is invoked 18 times; once for each IMDCT 
				 * output sample. The IMDCT synthesis stage generates
				 * 18-points (or 3 x 6-points for short-block mode) on
				 * each of the 32 PQMF channels. Then, the PQMF synthesis 
				 * takes 32-inputs across frequency, 1 from each IMDCT
				 * output, and synthesizes 32 time-domain outputs.  The
				 * process is repeated 18 times, once for each sample in
				 * the current block of IMDCT outputs, resulting in a
				 * total output of 576 time-domain output samples. Mode
				 * "2" is selected to force interleaved pcm outputs for
				 * both the one- and two-channel cases to force data
				 * alignment that is compatible with many audio devices.
				 * In the one channel case, the sound rendering routine
				 * could replicate channel 0 data on channel 1. In the
				 * PQMF loop below, the parameter "offset" selects the
				 * block of IMDCT coefficients associated with the current 
				 * channel, the index k selects the set of 32 synthesis
				 * PQMF inputs associated with the current block of 32
				 * time-domain output samples, and the index j selects the 
				 * block of 32 time-domain pcm output samples associated
				 * with the current set of PQMF inputs.  The parameter
				 * pcmblock selects the block of PCM output samples
				 * associated with the current granule. 
				 */
				offset = ch * IPP_MP3_GRANULE_LEN;
				pcmblock = gr * IPP_MP3_GRANULE_LEN * MAX_CHAN;
				for (j = k = 0; k < IMDCT_BLK_LEN;
				     k++, j += (PQMF_BANDS * MAX_CHAN))
					ippsSynthPQMF_MP3_32s16s(&(Xs[(k * PQMF_BANDS) + offset]),
								 &(pcmaudio[j + ch + pcmblock]),
								 &(PQMF_V_Buf[ch * IPP_MP3_V_BUF_LEN]),
								 &(PQMF_V_Indx[ch]), 2);
			}	/* end of the channel loop, part B */

		}		/* end of the granule processing loop */

	} /* end of the main data OK clause */
	else {
		return (MP3_FRAME_UNDERRUN);
	}
	return (MP3_FRAME_COMPLETE);
}


/*******************************************************************************
// Name: InitMP3Decoder
//
// Description:		 Initialize decoder state variables 
//
// Input Arguments:  s	- Pointer to the MP3 decoder state
//		 bs	- Pointer to the MP3 input bit stream 
//
// Output Arguments: None			
//
// Returns: None.
//
******************************************************************************/
int
InitMP3Decoder(MP3DecoderState * s, MP3BitStream * bs)
{
	IppMP3SideInfo *SideInfoPtr;
	int i, j;

	/***************************************
	 Init actions are grouped by primitive
	****************************************/

	/***********************************
	 1. ippsUnpackFrameHeader_MP3 
	 ***********************************/
	/*
	 * Clear frame header 
	 */
	(s->FrameHdr).id = 0;
	(s->FrameHdr).layer = 0;
	(s->FrameHdr).protectionBit = 0;
	(s->FrameHdr).bitRate = 0;
	(s->FrameHdr).samplingFreq = 0;
	(s->FrameHdr).paddingBit = 0;
	(s->FrameHdr).privateBit = 0;
	(s->FrameHdr).mode = 0;
	(s->FrameHdr).modeExt = 0;
	(s->FrameHdr).CRCWord = 0;

	/***********************************
	 2. ippsUnpackSideInfo_MP3 
	 ***********************************/
	/*
	 * Clear table of side information 
	 */
	SideInfoPtr = &((s->SideInfo)[0][0]);
	for (i = 0; i < MAX_GRAN; i++)
		for (j = 0; j < MAX_CHAN; j++) {
			SideInfoPtr->part23Len = 0;
			SideInfoPtr->bigVals = 0;
			SideInfoPtr->globGain = 0;
			SideInfoPtr->sfCompress = 0;
			SideInfoPtr->winSwitch = 0;
			SideInfoPtr->blockType = 0;
			SideInfoPtr->mixedBlock = 0;
			SideInfoPtr->pTableSelect[0] = 0;
			SideInfoPtr->pTableSelect[1] = 0;
			SideInfoPtr->pTableSelect[2] = 0;
			SideInfoPtr->pSubBlkGain[0] = 0;
			SideInfoPtr->pSubBlkGain[1] = 0;
			SideInfoPtr->pSubBlkGain[2] = 0;
			SideInfoPtr->reg0Cnt = 0;
			SideInfoPtr->reg1Cnt = 0;
			SideInfoPtr->preFlag = 0;
			SideInfoPtr->sfScale = 0;
			SideInfoPtr->cnt1TabSel = 0;
			SideInfoPtr++;
		}

	/***********************************
	 3. ippsUnpackScaleFactors_MP3_1u8s 
	 ***********************************/
	/*
	 * Clear scalefactors 
	 */
	for (i = 0; i < MAX_CHAN * IPP_MP3_SF_BUF_LEN; i++)
		(s->ScaleFactor)[i] = 0;

	/*
	 * Clear scalefactor select information 
	 */
	for (i = 0; i < MAX_CHAN * SCF_BANDS; i++)
		(s->Scfsi)[i] = 0;

	/*******************************
	 4. ippsHuffmanDecode_MP3_1u32s 
	 ********************************/
	/*
	 * Clear Huffman sybmol buffer and requantization I/O buffer 
	 */
	for (i = 0; i < MAX_CHAN * IPP_MP3_GRANULE_LEN; i++)
		(s->IsXr)[i] = 0;

	/*
	 * Clear NonZeroBounds 
	 */
	(s->NonZeroBound)[0] = (s->NonZeroBound)[1] = 0;

	/*******************************
	 5. ippsReQuantize_MP3_32s_I 
	 ********************************/
	/*
	 * Clear requantization work space buffer 
	 */
	for (i = 0; i < IPP_MP3_GRANULE_LEN; i++)
		(s->RequantBuf)[i] = 0;

	/*******************************
	 6. ippsMDCTInv_MP3_32s 
	 ********************************/
	/*
	 * Clear IMDCT output and overlap-add buffers 
	 */
	for (i = 0; i < IPP_MP3_GRANULE_LEN; i++) {
		(s->Xs)[i] = (s->Xs)[i + IPP_MP3_GRANULE_LEN] = 0;
		(s->OverlapAddBuf)[i] = (s->OverlapAddBuf)[i + IPP_MP3_GRANULE_LEN] = 0;
	}
	(s->PreviousIMDCT)[0] = (s->PreviousIMDCT)[1] = 0;

	/*******************************
	 7. ippsSynthPQMF_MP3_32s16s 
	 ********************************/
	/*
	 * Clear Synthesis PQMF memory and V buffer indices 
	 */
	for (i = 0; i < MAX_CHAN * IPP_MP3_V_BUF_LEN; i++)
		(s->PQMF_V_Buf)[i] = 0;
	(s->PQMF_V_Indx)[0] = (s->PQMF_V_Indx)[1] = 0;

	/*******************************
	 Main Data Decoding Buffer 
	 ********************************/
	/*
	 * Clear main data decoding buffer 
	 */
	for (i = 0; i < MAIN_DATA_BUF_SIZE; i++)
		(s->MainDataBuf)[i] = 0;

	/*
	 * Initialize Main Data end of buffer pointer 
	 */
	s->MainDataEnd = 0;

	/**********************************
	 Application interface parameters 
	 **********************************/
	s->Channels = 0;
	s->pcmLen = 0;

	/********************************
	 Init raw MP3 stream ring buffer 
	 ********************************/
	bs->Head = 0;
	bs->Tail = 0;
	bs->Len = STREAM_BUF_SIZE;	/* Must be radix-2 (see header file) */

	/*
	 * Success 
	 */
	return (1);
}

/********************************************
 Bit Stream and Ring Buffer Helper Functions 
*********************************************/

/*******************************************************************************
// Name: SeekMP3Sync
//
// Description: Locate the next byte-alinged sync word in the raw mp3 stream. 
//
// Input Arguments:  bs	- Pointer to the raw mp3 bit stream structure  
//
// Output Arguments: None				
//
// Returns: Status code	- 1 = success, 0 = sync not found.
*******************************************************************************/
int
SeekMP3Sync(MP3BitStream * bs)
{
	int SyncFound = FALSE;
	int BufEmpty = FALSE;
	Ipp16u Val = 0x0000;
	unsigned int Mask;

	/*
	 * Exploit wrap-around modulo arithmetic for ring-buffer wrap around 
	 */
	Mask = (bs->Len) - 1;

	/*
	 * Scan input bit stream for a byte-alinged sync word Termminate scan 
	 * when sync found or end of buffer reached 
	 */
	while (!(SyncFound || BufEmpty)) {
		Val <<= BITS_PER_BYTE;
		Val |= (bs->Stream)[bs->Head];
		BufEmpty = (bs->Head == bs->Tail);
		bs->Head = (bs->Head + 1) & Mask;
		SyncFound = ((((Val & SYNC_WORD_MASK) == SYNC_WORD)
			      || (((Val >> 4) & SYNC_WORD_MASK) == SYNC_WORD)));
	}

	/*
	 * If sync is found (whether buffer empty or not), advance Head
	 * pointer to first sync byte 
	 */
	/*
	 * If sync not found and buffer empty, set head pointer equal to tail
	 * pointer 
	 */
	if (SyncFound)
		bs->Head = (bs->Head - 2) & Mask;
	else
		bs->Head = (bs->Head - 1) & Mask;
	return (SyncFound);
}

/*******************************************************************************
// Name: GetStreamBytes
//
// Description: Extract N bytes from the ring buffer containing the raw mp3 bitstream.
//
// Input Arguments:  bs	- Pointer to the mp3 input bit stream structure
//		 N	- Requested number of bytes to extract 
//
// Output Arguments: Buf - Output buffer for the extracted bytes
//
// Returns: i	- i==number of bytes actually extracted (i<=N).
//		  i<N:  typically indicates ring buffer underrun;
//			application must load additional bytes into
//			the ring buffer and retry decoder call.
//		  i==N: desired outcome.
*******************************************************************************/
int
GetStreamBytes(MP3BitStream * bs, Ipp8u * Buf, int N)
{
	int i;
	int EndOfStream = FALSE;
	unsigned int Mask;

	/*
	 * Exploit wrap-around modulo arithmetic for ring-buffer wrap around 
	 */
	Mask = (bs->Len) - 1;

	for (EndOfStream = (bs->Head == bs->Tail), i = 0; (i < N) && (!EndOfStream); i++) {
		Buf[i] = (bs->Stream)[bs->Head];
		EndOfStream = (bs->Head == bs->Tail);
		bs->Head = (bs->Head + 1) & Mask;
	}

	/*
	 * Adjust head pointer for end of stream condition 
	 */
	if (EndOfStream)
		bs->Head = bs->Tail;
	return (i);
}

/*******************************************************************************
// Name: StreamBytesRemaining
//
// Description:	 Indicates the number of bytes remaining in the ring-buffered mp3 stream.
//
// Input Arguments:  bs	- Pointer to the mp3 input bit stream structure
//
// Output Arguments: None				
//
// Returns: Result - Number of bytes remaining in the stream ring buffer
*******************************************************************************/
int
StreamBytesRemaining(MP3BitStream * bs)
{
	if (bs->Head < bs->Tail)
		return (bs->Tail - bs->Head + 1);
	else
		return (bs->Len - bs->Head + bs->Tail + 1);
}

