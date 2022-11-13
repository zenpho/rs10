# rs10 is a tool to extract roland s10 sample data to wav

The S10 (and the sister products MKS100 and S220) were released before the MIDI SDS (Sample Dump Standard) was developed. This tool helps extract samples from this hardware.

The current version is 0.6. The tool is for command line use. The code uses standard libraries only. A MacOS Mach-O 64-bit executable (x86_64) is available.

To use the tool, provide a path to a .syx input file on disk, and a desired path to a wav output file. The .syx input file must contain raw MIDI bytes from a "one way". Standard MIDI .mid input is not supported. The tool will produce a 16-bit signed integer 32000Hz single (mono) channel wav file containing all PCM sample data found in the .syx input file.

#Usage 
$ rs10 in.syx out.wav
