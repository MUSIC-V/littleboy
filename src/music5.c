// MUSIC V driver program
// calls pass1, pass2, pass3 and raw2wav to produce an output
// (c) V Lazzarini, 2022
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE

#include <stdlib.h>
#include <stdio.h>
#define BUFSIZE 1024

extern int pass1_();
extern int pass2_(int *);
extern int pass3_();
extern int raw2wav(const char *, const char *, int, int); 

typedef struct _params_ {
  const char *score;
  const char *fname;
  int  pass;
} params;

int argdecode(int argc, const char *argv[], params *p) {
  for(int i = 1; i < argc; i++) {
    if(*argv[i] == '-') {
      if(*(argv[i]+1) == 'p') {
	if(*(argv[i]+2) == '\0') i++;
	p->pass = strtol(argv[i],NULL,0);
      }
      if(*(argv[i]+1) == 'h') {
        printf("usage: %s [-p pass] [score] [output.wav] \n", argv[0]);
        return 0;
      }
    }
    else {
      if(p->score == NULL) {
	p->score = argv[i];
      }
      else {
	p->fname = argv[i];
      }
    }
  }
  return 1;
}

int main(int argc, const char *argv[]) {
  params p = { NULL, NULL, 0 };
  if (argdecode(argc,argv,&p)) {
    FILE *fin,*fout;
    int ret = 0;
    int subr = 0;
    char buffer[BUFSIZE];
    if(p.pass == 0 || p.pass == 1) {
      if(p.score) {
	fin = fopen(p.score,"r");
	if(fin) {
	  fout = fopen("score","w");
          fprintf(fout, " COMMENT score: %s\n", argv[1]);
	  if(fout) {
	    do {
	      ret = fread(buffer,1,BUFSIZE,fin);
	      fwrite(buffer,1,ret,fout);
	    } while(ret);
	    fclose(fout);
	  }
	  else {
	    fclose(fin);
	    printf("could not open score file for writing\n");
	    return -1;
	  }
	}
	else {
	  printf("could not open %s for reading\n", argv[1]);
	  return -1;
	}
	fclose(fin);
      } 
      ret = pass1_();
    }
    if(p.pass == 0 || p.pass == 2) {
      if(ret == 0){
	fin = fopen("score","r");
	if(fin) {
	  do {
	    ret = fscanf(fin," COMMENT: CONVT %d ; \n", &subr);
	    fscanf(fin, "%s\n",buffer);
	  } while(!feof(fin) || ret == 0);
	  printf(" CONVT: %d \n", subr);
	  fclose(fin);      
	}
	else {
	  fclose(fin);
	  printf("could not open score file for writing\n");
	  return -1;
	}
	ret = pass2_(&subr);
      }
      else {
	printf("pass 1 failed\n");
	return 1;
      }
    }
    if(p.pass == 0 || p.pass == 3) {
      if(ret == 0) ret = pass3_();
      else {
	printf("pass 2 failed\n");
	return 2;
      } 
      if(ret == 0) {
	int chn = 1;
	int sr = 44100;
	fin = fopen("snd_params.txt","r");
	if(fin) {
	  fscanf(fin, "%d", &chn);
	  fscanf(fin, "%d", &sr);
	  fclose(fin);
	}
	printf(" PASS III completed successfully\n"
	       "Created snd.raw (32-bit floats, sr = %d KHz, %s)\n", sr,
	       chn == 2 ? "stereo" : "mono");
	ret = raw2wav("snd.raw", p.fname ? p.fname : "out.wav", chn == 2 ? 2 : 1, sr);
	if(ret) {
	  printf("Failed to convert snd.raw to %s\n", argv[2]);
	  return -1;
	}
	return 0;
      } else {
	printf("pass 3 failed\n");
	return 3;
      }
    }
    return 0;
  }  else return -1;
}


				    
