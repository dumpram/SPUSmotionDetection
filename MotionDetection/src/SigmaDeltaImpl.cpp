#include <SigmaDeltaImpl.h>

#define TV 4

static int t = 0;

void basicSigmaDeltaBS(Mat &I, Mat &M, Mat &V, Mat &E, int N) {
	uchar *I_p;
	uchar *M_p;
	uchar *V_p;
	uchar *E_p;
	uchar O;
	for(int i = 0; i < I.rows; i++) {
		I_p = I.ptr<uchar>(i);
		M_p = M.ptr<uchar>(i);
		V_p = V.ptr<uchar>(i);
		E_p = E.ptr<uchar>(i);
		for(int j = 0; j < I.cols; j++) {
			if (M_p[j] < I_p[j]) {
				M_p[j]++;
			} else if (M_p[j] > I_p[j]) {
				M_p[j]--;
			}
			O = abs(M_p[j] - I_p[j]);
			if (O != 0) {
				if (V_p[j] < N*O) {
					V_p[j]++;
				} else if (V_p[j] > N*O) {
					V_p[j]--;
				}
			}
			if (O < V_p[j]) {
				E_p[j] = (uchar)0;
				
			} else if (O > V_p[j]) {
				E_p[j] = (uchar)255;
			}
		}
	}
}

void zipfSigmaDeltaBS(Mat &I, Mat &M, Mat &V, Mat &E, int N) {
	uchar *I_p;
	uchar *M_p;
	uchar *V_p;
	uchar *E_p;
	uchar O;
	if (t > 255) {
		t = 0;
	}
	t++;
	int rank = (t % 256); 
	int pow2 = 1;
	int thres = 256;
	do {
		pow2 = pow2 * 2;
		thres = thres / 2; 
	} while((rank % pow2 == 0) && (thres > 1));
	for(int i = 0; i < I.rows; i++) {
		I_p = I.ptr<uchar>(i);
		M_p = M.ptr<uchar>(i);
		V_p = V.ptr<uchar>(i);
		E_p = E.ptr<uchar>(i);
		for(int j = 0; j < I.cols; j++) {
			if (V_p[j] > 256/pow2) {
				if (M_p[j] < I_p[j]) {
					M_p[j]++;
				} else if (M_p[j] > I_p[j]) {
					M_p[j]--;
				}
			}
			O = abs(M_p[j] - I_p[j]);
			if (O != 0) {
				if (t % TV == 0) {
					if (V_p[j] < N*O) {
						V_p[j]++;
					} else if (V_p[j] > N*O) {
						V_p[j]--;
					}
				}
			}
			if (O < V_p[j]) {
				E_p[j] = (uchar)0;
				
			} else if (O > V_p[j]) {
				E_p[j] = (uchar)255;
			}
		}
	}
}