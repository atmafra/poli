#include <stdio.h>
#include <stdlib.h>
#include "s_smptypes.h"
#include "s_files.h"
#include "s_samples.h"
#include "s_preemph.h"
#include "s_fft.h"
#include "s_window.h"
#include "s_cepstrum.h"


int
main (int argc, char **argv)
{
  index_list_type file_index, frames_index, fft_index, mfcc_index;
  char *input_file_name = "../teste/teste.wav";
  char *output_file_name = "../teste/tmp/teste_win_fft.dat";
  smp_index_pos input_file_pos;
  /*smp_num_samples cur_samp; */
  /*cmp_complex z_in, z_out; */
  /*smp_num_samples filter; */
  /*cmp_real f_lo, f_md, f_hi; */
  int exit_status;


  /* Nome do arquivo de entrada */
  if (argc > 1)
    input_file_name = argv[1];

  /* Nome do arquivo de saida */
  if (argc > 2)
    output_file_name = argv[2];

  /* Cria um índice para os arquivos de entrada */
  exit_status = create_index (&file_index, NULL);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro criando índice de arquivos\n");
      return EXIT_FAILURE;
    }

  /* Cria o índice para a FFT */
  exit_status = create_index (&fft_index, NULL);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro criando índice de FFT's\n");
      return EXIT_FAILURE;
    }

  /* Cria uma lista nova dentro do índice para o arquivo */
  exit_status =
    add_index_entry (&file_index, "Arquivo de entrada",
                     input_file_name, NULL, SMP_REAL, 0.0, 0.0, 0,
                     &input_file_pos, SMP_SET_CURRENT);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro criando lista para o arquivo\n");
      return EXIT_FAILURE;
    }

  /* Le o arquivo de entrada */
  exit_status = read_samples_file (&file_index, input_file_pos);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro lendo arquivo de dados\n");
      return EXIT_FAILURE;
    }

  /* Faz a pre-enfase */
  /*
     exit_status = spre_list_preemphasis (file_index.current->list, 0.95);

     if (exit_status != EXIT_SUCCESS)
     {
     fprintf (stderr, "Erro fazendo a pré-ênfase\n");
     return EXIT_FAILURE;
     }
   */


  /* Calcula o espectro de potencia */
  /*
     exit_status =
     apply_index_function (&fft_index, cmp_sqr, CMP_FCN_COMPLEX, 0);
     if (exit_status != EXIT_SUCCESS)
     {
     fprintf (stderr, "Erro calculando o espectro de potência\n");
     return EXIT_FAILURE;
     }
   */

  /* Windowing of the input signal */
  exit_status = swin_window (&file_index,
                             SWIN_YES,
                             0.95,
                             512,
                             64,
                             SMP_COMPLEX,
                             SMP_REAL,
                             SWIN_HAMMING,
                             0.0,
                             0.0,
                             0.0,
                             0.0, 0.0, SWIN_PURGE_ZERO_POWER, &frames_index);

  /* Faz a FFT do sinal */
  exit_status = sfft_exec_index (&frames_index,
                                 &frames_index,
                                 SFFT_FFT,
                                 SFFT_IN_PLACE, SFFT_DIRECT, SFFT_REAL);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro executando a FFT do sinal\n");
      return EXIT_FAILURE;
    }

  /* Espectro de potencia */
  exit_status =
    apply_index_function (&frames_index, cmp_sqr, CMP_FCN_COMPLEX, 0);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro calculando o espectro de potencia\n");
      return EXIT_FAILURE;
    }

  /* Bancos mel-frequency */
  exit_status = scep_mfcc_index (frames_index, 100.0, 32, &mfcc_index);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file: error calcutating MFCC's\n");
      return EXIT_FAILURE;
    }

  /* Espectro de potencia */
  exit_status =
    apply_index_function (&mfcc_index, cmp_rlg, CMP_FCN_REAL, 1, 10.0);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro calculando o espectro de potencia\n");
      return EXIT_FAILURE;
    }

  /* Executes the inverse DCT */
  exit_status =
    sfft_exec_index (&mfcc_index, &mfcc_index, SFFT_FCT, SFFT_IN_PLACE,
                     SFFT_DIRECT, SFFT_REAL);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "scep_mfcc_file: error executing the DCT\n");
      return EXIT_FAILURE;
    }


  /* Escreve o arquivo de entrada */
  exit_status = write_index (mfcc_index,
                             output_file_name,
                             SMP_DONT_WRITE_INDEX,
                             SMP_WRITE_LISTS,
                             SMP_WRITE_NORM_TIME,
                             SMP_WRITE_MANY_FILES,
                             SMP_BREAK_LINES, SMP_YES, SMP_NO);

  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro escrevendo arquivo de saída %s\n",
               output_file_name);
      return EXIT_FAILURE;
    }


  /*
     z_in.re = 1.0;
     z_in.im = 0.0;
     for (cur_samp = 0; cur_samp <= 512; cur_samp++)
     {
     exit_status = swin_windowed_value (z_in,
     cur_samp,
     512,
     SWIN_BLACKMAN,
     SMP_REAL,
     0.0,
     0.0,
     0.0,
     0.0,
     0.0,
     &z_out);

     printf ("%ld %f\n", cur_samp, z_out.re);
     }
   */

  /*
     for (filter = 1; filter <= 32; filter++)
     {
     exit_status = scep_mel_scale (100.0, 32, filter, &f_lo, &f_md, &f_hi);
     if (exit_status != EXIT_SUCCESS)
     {
     fprintf (stderr, "Erro calculando frequencias do filtro %ld\n", filter);
     return EXIT_FAILURE;
     }
     printf ("%f 0.0\n%f 1.0\n%f 0.0\n\n", f_lo, f_md, f_hi);
     }
   */

  return EXIT_SUCCESS;
}
