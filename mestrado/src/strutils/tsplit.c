#include <stdio.h>
#include "strutils.h"

int
main (int argc, char **argv)
{
  char *f0 = "/home/";
  char *f1 = "mafra";
  char *f2 = ".txt";
  char *f3 = "/home/mafra";
  char *f4 = "mafra.txt";
  char *f5 = "/home/.txt";
  char *f6 = "/home/mafra.txt";
  char *f7 = "/home.0/mafra/";
  char *f8 = "/home.0/mafra.net.txt";
  char *f9 = (char *) NULL;
  char *f10 = "";
  char *f11 = "  /home / home . txt  ";
  char *f12 = "..........";
  char *f13 = "./././././";
  char *f14 = "usr/include/mafra.";
  char *dir = (char *) NULL;
  char *base = (char *) NULL;
  char *ext = (char *) NULL;


  printf ("%s\n", f0);
  split_file_name (f0, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    base = (char *) NULL;
  if (ext != (char *) NULL)
    ext = (char *) NULL;
  puts ("");

  printf ("%s\n", f1);
  split_file_name (f1, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f2);
  split_file_name (f2, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f3);
  split_file_name (f3, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f4);
  split_file_name (f4, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f5);
  split_file_name (f5, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f6);
  split_file_name (f6, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f7);
  split_file_name (f7, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f8);
  split_file_name (f8, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f9);
  split_file_name (f9, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f10);
  split_file_name (f10, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f11);
  split_file_name (f11, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f12);
  split_file_name (f12, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f13);
  split_file_name (f13, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  printf ("%s\n", f14);
  split_file_name (f14, &dir, &base, &ext);
  printf ("dir = '%s'\nbase = '%s'\next = '%s'\n", dir, base, ext);
  if (dir != (char *) NULL)
    free (dir);
  if (base != (char *) NULL)
    free (base);
  if (ext != (char *) NULL)
    free (ext);
  puts ("");

  return 0;
}
