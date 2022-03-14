int fact(int n) {
  if (n == 0)
    return 1;
  int res = n * fact(n - 1);
  return res;
}
