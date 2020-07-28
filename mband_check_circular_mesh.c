#include <stdio.h>
#include <stdlib.h>
#define NODE 4

void *malloc2d(size_t size, int row, int col);

int main(void)
{
    // 円メッシュでのnboolの作成
    int nc = 4;
    int nl = 3;
    int r_divide = 3;

    int nc_1 = nc + 1;

    int nelement = (nc * nc) + ((4 * nc) * (nl + r_divide));
    int nnode = ((nc + 1) * (nc + 1)) + ((4 * nc) * (nl + r_divide));

    int np[4], ie;

    int **nbool;
    nbool = (int **)malloc2d(sizeof(int), nelement, 4);

    // 核部周囲の衛星節点の番号と円周方向の節点番号の対応づけ
    int nr[(nc_1 * 2) + (nc_1 - 2) * 2];
    for(int i=0;i<((nc/2)-1);i++){
        nr[i] = nc_1 * ((nc/2-1)-i);
    }
    for(int i=0;i<nc_1;i++){
        nr[(nc/2-1)+i] = i;
    }
    for(int i=0;i<nc;i++){
        nr[((nc/2)*3+i)] = nc_1*(i+2)-1; 
    }
    for(int i=0;i<nc;i++){
        nr[(nc/2*5)+i] = (nc_1*nc_1-2)-i;
    }
    for(int i=0;i<nc/2;i++){
        nr[nc/2*7+i] = nc_1*(nc_1-2-i);
    }

    for(int j=0; j<nc; j++){
        for(int i=0; i<nc; i++){
            ie = i + nc * j;
            np[0] = i + nc_1 * j;
            np[1] = np[0] + 1;
            np[3] = np[1] + nc;
            np[2] = np[3] + 1;

            nbool[ie][0] = np[0];
            nbool[ie][1] = np[1];
            nbool[ie][2] = np[2];
            nbool[ie][3] = np[3];
        }
    }

    // 円周部分のnboolの作成
    for(int j=0;j<(nl+r_divide);j++){//物質1・2の半径方向ループ
        for(int i=0;i<4*nc;i++){// 円周方向のループ
            ie = (nc * nc) + i + (4 * nc * j);
            if(j == 0){
                np[0] = nr[i];
                if(i == 0){
                    np[1] = nr[(4 * nc) - 1];
                }else{
                    np[1] = nr[i - 1];
                }
            }else{
                np[0] = (nc_1 * nc_1) + i + 4 * nc * (j - 1);
                if(i == 0){
                    np[1] = (nc_1 * nc_1) + (4 * nc * j) - 1;
                }else{
                    np[1] = np[0] - 1;
                }
            }
            if(i == 0){
                np[2] = (nc_1 * nc_1) + (4 * nc * (j+1)) - 1;
            }else{
                np[2] = (nc_1 * nc_1) + i - 1 + (4 * nc * j);
            }
            np[3] = (nc_1 * nc_1) + i + (4 * nc * j);

            nbool[ie][0] = np[0];
            nbool[ie][1] = np[1];
            nbool[ie][2] = np[2];
            nbool[ie][3] = np[3];
        }
    }

    for(int i=0; i<nelement; i++){
        printf("%d:( %d, %d, %d, %d)\n", i, nbool[i][0], nbool[i][1], nbool[i][2], nbool[i][3]);
    }

    // ここから全体行列の作成
    int glb_mt[nnode][nnode]; // 全体行列
    int locl_mt[nelement][NODE][NODE];

    // 全体行列の初期化
    for(int i=0; i<nnode; i++){
        for(int j=0; j<nnode; j++){
            glb_mt[i][j]=0;
        }
    }

    // 局所行列の作成
    for(ie=0; ie<nelement; ie++){
        for(int i=0; i<NODE; i++){
            for(int j=0; j<NODE; j++){
                locl_mt[ie][i][j] = 1;
            }
        }
    }

    // 全体行列の作成
    for(ie=0; ie<nelement; ie++){
        for(int i=0; i<NODE; i++){
            int nu1 = nbool[ie][i];
            for(int j=0; j<NODE; j++){
                int nu2 = nbool[ie][j];
                glb_mt[nu1][nu2]+=locl_mt[ie][i][j];
            }
        }
    }

    for(int i=0; i<nnode; i++){
        for(int j=0; j<nnode; j++){
            printf("%3d", glb_mt[i][j]);
        }
        printf("\n");
    }

    // 全体行列の帯幅と半帯幅を求める
    int non_zero_range = -1; // 帯幅
    int max_band_range_i = -1; // 最大帯幅の行
    for(int i=0; i<nnode; i++){ // 行シフト
        int flag_non_zero_start = -1;
        int non_zero_l = -1;
        int non_zero_r = -1;
        for(int j=0; j<nnode; j++){ // 列シフト
            // ある行において，一番左非零成分から一番右の非零成分までの長さを求める
            if((glb_mt[i][j] != 0)&&(flag_non_zero_start == -1)){
                non_zero_l = j;
                flag_non_zero_start = 1; // 一番左の非零成分の場所を保存
            }
            if(glb_mt[i][j] != 0){
                non_zero_r = j;
            }
        }
        if(non_zero_range < (non_zero_r - non_zero_l)+1){
            non_zero_range = non_zero_r - non_zero_l+1;
            max_band_range_i = i; // 最大帯幅の行の場所を保存(0から数えている)
        }

    }

    printf("最大帯幅の行 = %d, 帯幅 = %d\n", max_band_range_i, non_zero_range);


    free(nbool);

    return 0;
}

void *malloc2d(size_t size, int row, int col)
{
  char **a, *b;
  int t = size * col;
  int i;

  a = (char**)malloc((sizeof(*a) + t) * row);
  if (a) {
      b = (char*)(a + row);
      for (i = 0; i < row; i++) {
          a[i] = b;
          b += t;
      }
      return a;
  }
  return NULL;
}