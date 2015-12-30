#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../include/s_complex.h"
#include "../include/s_fft.h"
#include "../include/s_window.h"
#include "../include/s_fmt_wav.h"

int
main ()
{

  /*
     register int i;
     double y;
     char buf[100];
     double lower_bound;
     double upper_bound;
     long segments;
     int method;
     cmp_complex zvalue;
     FILE *wav_FILE;
     swav_type wav_file;
     int detected;
     list_file_type file_type;
     smp_num_samples samples, reversed_samples, N, Npower;
   */
  int exit_status;
  index_list_type index, win_index, fft_index;
  smp_index_pos signal_pos;     /*, fft_pos; */
  char *sigfilename = "data/voice/numeros/atm-0-22khz-8b.wav";
  /*char *sigfilename = "data/voice/numeros/lgc-0-44khz-16b-st.wav"; */
  /*char *sigfilename = "data/hun-1.txt"; */


  /*
   * Test for integral
   * 
   printf ("limite inferior = ");
   scanf ("%s", buf);
   lower_bound = strtod (buf, NULL);

   printf ("limite superior = ");
   scanf ("%s", buf);
   upper_bound = strtod (buf, NULL);

   printf ("número de segmentos = ");
   scanf ("%s", buf);
   segments = strtol (buf, NULL, 10);

   printf ("método de integração: ");
   scanf ("%s", buf);
   method = (int) strtol (buf, NULL, 10);

   y = func_integral (lower_bound, upper_bound, segments, sin, method);

   printf ("integral = %f\n", y);
   *
   */

  /*
   * Test for samples

   create_list (&list);

   exit_status = add_index_entry (&list, "Fisk-1", "data/fisk-1.txt", &p);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro em add_index_entry para a lista 1\n");
   exit (EXIT_FAILURE);
   }

   exit_status = add_index_entry (&list, "Hun-1", "data/hun-1.txt", &p);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro em add_index_entry para a lista 2\n");
   exit (EXIT_FAILURE);
   }

   exit_status = add_index_entry (&list, "Perkin2", "data/perkin2.txt", &p);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro em add_index_entry para a lista 3\n");
   exit (EXIT_FAILURE);
   }

   exit_status = add_index_entry (&list, "Stin6", "data/stin6.txt", &p);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro em add_index_entry para a lista 4\n");
   exit (EXIT_FAILURE);
   }



   exit_status = read_samples_file (&list, 1);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro lendo arquivo de amostras da lista 1\n");
   exit (EXIT_FAILURE);
   }

   exit_status = read_samples_file (&list, 2);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro lendo arquivo de amostras da lista 2\n");
   exit (EXIT_FAILURE);
   }

   exit_status = read_samples_file (&list, 3);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro lendo arquivo de amostras da lista 3\n");
   exit (EXIT_FAILURE);
   }

   exit_status = read_samples_file (&list, 4);
   if (exit_status != EXIT_SUCCESS)
   {
   fprintf (stderr, "erro lendo arquivo de amostras da lista 3\n");
   exit (EXIT_FAILURE);
   }

   write_list (list, "data/teste.txt", SMP_YES);
   */

  /*
     add_index_entry (&list, "Lista 1", "data/lista1.txt", &signal_pos);
   */

  /* FFT */
  /*
     puts ("Vai executar a FFT...");
     exit_status = sfft_exec (&index, signal_pos, &fft_pos, SMP_NO);
     if (exit_status != EXIT_SUCCESS)

     {
     fprintf (stderr, "erro executando fft\n");
     exit (EXIT_FAILURE);
     }
     puts ("Executou a FFT!");
     exit_status = calculate_list_statistics (&(index.current->list));
   */

  /* FFT DA FFT */
  /*
     puts ("Vai executar a FFT da FFT...");
     signal_pos = fft_pos;
     exit_status = sfft_exec (&index, signal_pos, &fft_pos, SMP_NO);
     if (exit_status != EXIT_SUCCESS)

     {
     fprintf (stderr, "erro executando fft\n");
     exit (EXIT_FAILURE);
     }
     puts ("Executou a FFT da FFT!");
   */

  /* Calculates the FFT statistics */
  /*
     exit_status = calculate_list_statistics (&(index.current->list));

     write_index_list (index, "result.txt", SMP_NO);
     switch_entries (&index, 2, 3);
     write_index_list (index, "result2.txt", SMP_NO);
     12345678901234567890123456789012345678901234567890123456789012345678901234567890
   */


/*==============================================================================
  swin_window (&index, 1, SWIN_YES, 0.95, 4096, 1024, SWIN_HAMMING, 20.0,
	       &win_index);

  write_index_list (index, "index.txt", SMP_YES, SMP_YES, SMP_NO);
  write_index_list (win_index, "win_index.txt", SMP_YES, SMP_YES, SMP_NO);

  exit_status = sfft_exec_index (&win_index, &fft_index, SMP_YES, SMP_NO);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "teste: problemas na FFT do índice\n");
      exit (EXIT_FAILURE);
    }

  write_index_list (fft_index, "fft_index.txt", SMP_YES, SMP_YES, SMP_NO);
=========================================================================*/

  /* Criacao do indice do arquivo de entrada */
  puts ("criando indice...");
  exit_status = create_index_list (&index, NULL);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "Erro criando indice\n");
      exit (EXIT_FAILURE);
    }

  /* Criacao no indice de uma lista (entrada) para o arquivo */
  puts ("adicionando entrada...");
  exit_status =
    add_index_entry (&index, "Lista 1", sigfilename, NULL, &signal_pos);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro em add_index_entry para a lista %d\n",
               signal_pos);
      exit (EXIT_FAILURE);
    }

  /* Leitura do arquivo de entrada */
  puts ("lendo arquivo de amostras...");
  exit_status = read_samples_file (&index, signal_pos);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro lendo arquivo de amostras da lista 3\n");
      exit (EXIT_FAILURE);
    }

  /* Execucao do janelamento sobre a lista de entrada */
  puts ("executando janelamento");
  exit_status =
    swin_window (&index, 1, SWIN_YES, 0.95, 512, 256, SWIN_HAMMING, 20.0,
                 &win_index);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro executando janelamento\n");
      return (EXIT_FAILURE);
    }

  /* FFT do sinal janelado */
  puts ("executando fft do sinal janelado");
  exit_status = sfft_exec_index (&win_index, &fft_index, SMP_YES, SMP_NO);
  if (exit_status != EXIT_SUCCESS)
    {
      fprintf (stderr, "erro executando FFT\n");
      exit (EXIT_FAILURE);
    }

  write_index_list (index, "index.txt", SMP_YES, SMP_YES, SMP_NO);
  write_index_list (fft_index, "fft_index.txt", SMP_YES, SMP_YES, SMP_NO);

  exit (EXIT_SUCCESS);
}
