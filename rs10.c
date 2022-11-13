/*
	extract roland s10 system exclusive dump sample data 
	usage: rs10 in.syx out.wav
	v0.7
	by zenpho@zenpho.co.uk
*/

#include <stdio.h>

FILE* inFile = NULL;
FILE* outFile = NULL;

/** 
	usage()
	called to display example valid command line argument usage 
*/
void usage(char** argv)
{
	printf("rs10 v0.7 - by zenpho@zenpho.co.uk\n");
	printf("usage %s in.syx out.wav\n", argv[0]);	
}

/**
	handleDT1()
	called from handleSysex()
	processes roland s10 sample dump DT1 msg
	reads address bytes from inFile   - currently unused
	reads also data bytes from inFile
	writes 12-bit data (range 0 to 4095) to outFile as 'int'
*/
void handleDT1( long inFileLen )
{
	unsigned char val_a = 0; // upper data byte
	unsigned char val_b = 0; // lower data byte
	int val = 0;             // assembled 12-bit value

	long inFilePos = ftell( inFile );

	unsigned char addr_a = fgetc( inFile );
	unsigned char addr_b = fgetc( inFile );
	unsigned char addr_c = fgetc( inFile );
	inFilePos += 3;
	printf("---addr %x %x %x--- ", addr_a, addr_b, addr_c);

	if( addr_a == 1 )
	{
		printf("not sample data ");
		return;
	}

	while( !feof( inFile ) )
	{
		val_a = fgetc( inFile );		
		if( val_a == 0xF7 ) break;

		val_b = fgetc( inFile );
		if( val_b == 0xF7 ) break;

		// assemble 7-bit values to one 12-bit value, then shift to 16-bit range
		int val = ( (val_a << 5) + (val_b >> 2) ) << 4;
		
		putw( val, outFile );
	}
	printf("\n");
}

/** 
	handleSysex()
	called when system exclusive start byte is encountered
	reads data from inFile
	calls handleDT1() if sysex command is detected
*/
void handleSysex( long inFileLen )
{
	unsigned char val = 0;
	unsigned char buf[4] = { 0, 0, 0, 0 }; // data from inFIle
	unsigned char* buf_w = buf;            // ringbuffer writepos

	long inFilePos = ftell( inFile );
	//printf("sysex message at offset %ld ", inFilePos );

	while( !feof( inFile ) )
	{
		val = fgetc( inFile );
		if( val == 0xF7 ) break;  // end of system exclusive

		// buffer contains previous values read from inFile
		buf[0] = buf[1];
		buf[1] = buf[2];
		buf[2] = buf[3];
		buf[3] = val;

		// we care about the following message sequence - fallthru
		if( buf[0] == 0x41 ) 
		// ------ any channel ------ // if( buf[1] == 0x00 ) // only channel '1'
		if( buf[2] == 0x10 )
		if( buf[3] == 0x12 )
		{
			handleDT1( inFileLen );
		}		
	}
	printf("\n");
}

/**
	main()
	crude command line handling
	opens file handles to inFile and outFile
	loop ingests bytes from inFile
	crude wav header to outFile 
	writing outFile sample data is handled elsewhere
*/
int main(int argc, char** argv)
{	
	if( argc != 3 ) 
	{
		usage( argv );
		return -1;
	}

	printf("infile:%s outfile:%s\n", argv[1], argv[2]);

	// output filepath is SECOND argument
	char* outFilePath = argv[2];
	outFile = fopen( outFilePath, "wb" );
	if( outFile == NULL )
	{
		printf("error opening output %s\n", outFilePath);
		return -1;
	}

	// input filepath is FIRST argument
	char* inFilePath = argv[1];
	inFile = fopen( inFilePath, "rb" );
	if( inFile == NULL )
	{
		printf("error opening input %s\n", inFilePath);
		return -1;
	}

	// determine and report input file length
	fseek( inFile, 0, SEEK_END );          
   	long inFileLen = ftell( inFile );
	rewind( inFile );
	printf("reading %ld bytes from %s...\n", inFileLen, inFilePath);

	// crudely write RIFF WAVE header to output file
	const unsigned short NUM_CHANS = 1;
	const unsigned short BYTES_PER_SAMPLE = 2;
	const unsigned short BITS_PER_SAMPLE = 16;

	fwrite( "RIFF", sizeof(char), 4, outFile );
	putw( 36, outFile );
	fwrite( "WAVE", sizeof(char), 4, outFile );
	fwrite( "fmt ", sizeof(char), 4, outFile );
	putw( 16, outFile );
	fwrite( &NUM_CHANS, 2, 1, outFile );
	fwrite( &NUM_CHANS, 2, 1, outFile );
	putw( 32000 , outFile );
	putw( 2 * 32000, outFile );
	fwrite( &BYTES_PER_SAMPLE, 2, 1, outFile );
	fwrite( &BITS_PER_SAMPLE, 2, 1, outFile );
	fwrite( "data", sizeof(char), 4, outFile );

	// read input file
	unsigned char val = 0;
	while( !feof( inFile ) )
	{
		val = fgetc( inFile );
		if( val == 0xF0 ) handleSysex( inFileLen );
	}

	fclose( inFile );
	fclose( outFile );
	printf("done\n");

	return 0;
}
