#include "../include/twofast.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

float get_distance_2d(float x1, float y1, float x2, float y2){
  /* Distance between two points in 3D coordinates.

  Parameters
  ----------
  x1, y1 : float
    Coordinates of points 1.
  x2, y2 : float
    Coordinates of points 2.
  
  Returns
  -------
  dist : float
    Distance between points.
  */
  float dist;
  dist = sqrt(pow(x1 - x2, 2.) + pow(y1 - y2, 2.));
  return dist;
}

float get_distance_3d(float x1, float y1, float z1, float x2, float y2, float z2){
  /* Distance between two points in 3D coordinates.

  Parameters
  ----------
  x1, y1, z1 : float
    Coordinates of points 1.
  x2, y2, z2 : float
    Coordinates of points 2.
  
  Returns
  -------
  dist : float
    Distance between points.
  */
  float dist;
  dist = sqrt(pow(x1 - x2, 2.) + pow(y1 - y2, 2.) + pow(z1 - z2, 2.));
  return dist;
}

float get_distance_tomo(float phi1, float theta1, float phi2, float theta2){
  /* Great angle distance between two points on a sphere.

  Parameters
  ----------
  phi1, theta1 : float
    Coordinates of points 1 in radians.
  phi2, theta2 : float
    Coordinates of points 2 in radians.
  
  Returns
  -------
  dist : float
    Distance between points given in radians using Vincenty formula.
  */
  float dist;
  float delta_phi = abs(phi1 - phi2);
  float _temp1 = pow(cos(theta2-M_PI/2.)*sin(delta_phi), 2.) + pow(cos(theta1-M_PI/2.)*sin(theta2-M_PI/2.)-sin(theta1-M_PI/2.)*cos(theta2-M_PI/2.)*cos(delta_phi), 2.);
  _temp1 = sqrt(_temp1);
  float _temp2 = sin(theta1-M_PI/2.)*sin(theta2-M_PI/2.) + cos(theta1-M_PI/2.)*cos(theta2-M_PI/2.)*cos(delta_phi);
  dist = atan2(_temp1, _temp2);
  return dist;
}

void get_r(float r[], float minimum, float maximum, int numbins, bool uselog){
  /* Radial bin centres for normal and logged binnings.

  Parameters
  ----------
  r : float array
    Radial bin centres.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  */
  float dx, log10min, log10max, log10dx;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  for(int i = 0; i < numbins; i++){
    if(uselog == true){
      r[i] = pow(10., log10min + log10dx/2. + float(i)*log10dx);
    }
    else{
      r[i] = minimum + dx/2. + float(i)*dx;
    }
  }
}

void get_xi(long int num_data, long int num_rand, vector<float> &dd, vector<float> &dr, 
  vector<float> &rr, vector<float> &xi){
  /* Calculates the correlation function using the Landy-Szalay estimator.

  Parameters
  ----------
  num_data : long int
    The size of the data.
  num_rand : long int
    The size of the randoms.
  dd : vector
    Binned auto correlation for data.
  dr : vector
    Binned cross correlation for data with randoms.
  rr : vector
    Binned auto correlation for randoms.
  xi : vector
    Correlation function.
  */
  float nd, nr;
  nd = (float) num_data;
  nr = (float) num_rand;
  for(int i = 0; i < dd.size(); i++){
    xi[i] = 1. + pow(nr/nd, 2.)*(dd[i]/rr[i]) - 2.*(nr/nd)*(dr[i]/rr[i]);
  }
}

void get_dd_2d(vector<float> &dd, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x, vector<float> &y){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dd : vector
    Vector of auto pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x, y : vector
    2D coordinate positions.
  */
  long int k = 0, kmax, ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  kmax = x.size()*(x.size()-1)/2;

  for(long int i = 0; i <= x.size(); i++){
    for(long int j = 0; j <= i; j++){
      dist = get_distance_2d(x[i], y[i], x[j], y[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dd[ind] += 1.;
      }
      k += 1;
      if(k % (kmax/10) == 0){
        cout << "Auto Pairs = " << k/(kmax/10) << '/' << 10 << endl;
      }
    }
  }
  for(int i = 0; i < numbins; i++){
    dd[i] *= 2.;
  }
}

void get_dr_2d(vector<float> &dr, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x_data, vector<float> &y_data, vector<float> &x_rand, vector<float> &y_rand){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dr : vector
    Vector of cross pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x_data, y_data : vector
    3D coordinate positions of the data.
  x_rand, y_rand : vector
    3D coordinate positions of the randoms.
  */
  long int ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  for(long int i = 0; i < x_data.size(); i++){
    for(long int j = 0; j < x_rand.size(); j++){
      dist = get_distance_2d(x_data[i], y_data[i], x_rand[j], y_rand[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dr[ind] += 1.;
      }
    }
    if(i % ((x_data.size())/10) == 0){
      cout << "Cross Pairs = " << i/((x_data.size())/10) + 1 << '/' << 10 << endl;
    }
  }
}

void get_dd_3d(vector<float> &dd, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x, vector<float> &y, vector<float> &z){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dd : vector
    Vector of auto pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x, y, z : vector
    3D coordinate positions.
  */
  long int k = 0, kmax, ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  kmax = x.size()*(x.size()-1)/2;

  for(long int i = 0; i <= x.size(); i++){
    for(long int j = 0; j <= i; j++){
      dist = get_distance_3d(x[i], y[i], z[i], x[j], y[j], z[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dd[ind] += 1.;
      }
      k += 1;
      if(k % (kmax/10) == 0){
        cout << "Auto Pairs = " << k/(kmax/10) << '/' << 10 << endl;
      }
    }
  }
  for(int i = 0; i < numbins; i++){
    dd[i] *= 2.;
  }
}

void get_dr_3d(vector<float> &dr, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x_data, vector<float> &y_data, vector<float> &z_data,
  vector<float> &x_rand, vector<float> &y_rand, vector<float> &z_rand){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dr : vector
    Vector of cross pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x_data, y_data, z_data : vector
    3D coordinate positions of the data.
  x_rand, y_rand, z_rand : vector
    3D coordinate positions of the randoms.
  */
  long int ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  for(long int i = 0; i < x_data.size(); i++){
    for(long int j = 0; j < x_rand.size(); j++){
      dist = get_distance_3d(x_data[i], y_data[i], z_data[i], x_rand[j], y_rand[j], z_rand[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dr[ind] += 1.;
      }
    }
    if(i % ((x_data.size())/10) == 0){
      cout << "Cross Pairs = " << i/((x_data.size())/10) + 1 << '/' << 10 << endl;
    }
  }
}

void get_dd_tomo(vector<float> &dd, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &phi, vector<float> &theta){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dd : vector
    Vector of auto pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  phi, theta : vector
    Coordinate positions on a sphere.
  */
  long int k = 0, kmax, ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  kmax = phi.size()*(phi.size()-1)/2;

  for(long int i = 0; i <= phi.size(); i++){
    for(long int j = 0; j <= i; j++){
      dist = get_distance_tomo(phi[i], theta[i], phi[j], theta[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dd[ind] += 1.;
      }
      k += 1;
      if(k % (kmax/10) == 0){
        cout << "Auto Pairs = " << k/(kmax/10) << '/' << 10 << endl;
      }
    }
  }
  for(int i = 0; i < numbins; i++){
    dd[i] *= 2.;
  }
}

void get_dr_tomo(vector<float> &dr, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &phi_data, vector<float> &theta_data, vector<float> &phi_rand, vector<float> &theta_rand){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dr : vector
    Vector of cross pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  phi_data, theta_data : vector
    Tomographic coordinate positions of the data.
  phi_rand, theta_rand : vector
    Tomographic coordinate positions of the randoms.
  */
  long int ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  for(long int i = 0; i < phi_data.size(); i++){
    for(long int j = 0; j < phi_rand.size(); j++){
      dist = get_distance_tomo(phi_data[i], theta_data[i], phi_rand[j], theta_rand[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dr[ind] += 1.;
      }
    }
    if(i % ((phi_data.size())/10) == 0){
      cout << "Cross Pairs = " << i/((phi_data.size())/10) + 1 << '/' << 10 << endl;
    }
  }
}

void get_mpi_dd_2d(vector<float> &dd, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x, vector<float> &y, long int *partition_begin, long int *partition_end, 
  long int *partition_begin_i, long int *partition_begin_j, long int *partition_end_i,
  long int *partition_end_j, string *prefix){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dd : vector
    Vector of auto pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x, y : vector
    2D coordinate positions.
  partition_begin : long int
    Beginning of total partition count.
  partition_end : long int
    End of total partition count.
  partition_being_i, partition_begin_j : long int
    Index for rows and columns beginnings.
  partition_end_i, partition_end_j : long int
    Index for rows and columns endings.
  prefix : string
    Prefix to be printed to show 10% progress.
  */
  long int k = 0, ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  long int j_start, j_end;

  double part_total = (double) (*partition_end - *partition_begin);

  long int _part_total, _temp_begin, _temp_end;

  _temp_begin = (*partition_begin_i+1)*(*partition_begin_i)/2;
  _temp_end = (*partition_end_i+1)*(*partition_end_i)/2;

  _temp_begin -= *partition_begin_i - *partition_begin_j + 1;
  _temp_end -= *partition_end_i - *partition_end_j;

  _part_total = _temp_end - _temp_begin;

  for(long int i = *partition_begin_i; i <= *partition_end_i; i++){
    if((i == *partition_begin_i) && (i == *partition_end_i)){
      j_start = *partition_begin_j;
      j_end = *partition_end_j;
    }
    else if(i == *partition_begin_i){
      j_start = *partition_begin_j;
      j_end = i-1;
    }
    else if(i == *partition_end_i){
      j_start = 0;
      j_end = *partition_end_j;
    }
    else{
      j_start = 0;
      j_end = i-1;
    }
    for(long int j = j_start; j <= j_end; j++){
      dist = get_distance_2d(x[i], y[i], x[j], y[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dd[ind] += 1.;
      }
      k += 1;
      if(k % (_part_total/10) == 0){
        cout << *prefix << "Auto Pairs = " << k/(_part_total/10) << '/' << 10 << endl;
      }
    }
  }
  for(int i = 0; i < numbins; i++){
    dd[i] *= 2.;
  }
}

void get_mpi_dr_2d(vector<float> &dr, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x_data, vector<float> &y_data, vector<float> &x_rand, vector<float> &y_rand,
  long int *partition_begin, long int *partition_end, string *prefix){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dr : vector
    Vector of cross pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x_data, y_data : vector
    2D coordinate positions of the data.
  x_rand, y_rand : vector
    2D coordinate positions of the randoms.
  partition_begin : long int
    Beginning of total partition count.
  partition_end : long int
    End of total partition count.
  prefix : string
    Prefix to be printed to show 10% progress.
  */
  long int ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  for(long int i = *partition_begin; i < *partition_end; i++){
    for(long int j = 0; j < x_rand.size(); j++){
      dist = get_distance_2d(x_data[i], y_data[i], x_rand[j], y_rand[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dr[ind] += 1.;
      }
    }
    if((i - *partition_begin) % ((*partition_end - *partition_begin)/10) == 0){
      cout << *prefix << "Cross Pairs = " << (i - *partition_begin)/((*partition_end - *partition_begin)/10) + 1 << '/' << 10 << endl;
    }
  }
}

void get_mpi_dd_3d(vector<float> &dd, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x, vector<float> &y, vector<float> &z, long int *partition_begin,
  long int *partition_end, long int *partition_begin_i, long int *partition_begin_j,
  long int *partition_end_i, long int *partition_end_j, string *prefix){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dd : vector
    Vector of auto pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x, y, z : vector
    3D coordinate positions.
  partition_begin : long int
    Beginning of total partition count.
  partition_end : long int
    End of total partition count.
  partition_being_i, partition_begin_j : long int
    Index for rows and columns beginnings.
  partition_end_i, partition_end_j : long int
    Index for rows and columns endings.
  prefix : string
    Prefix to be printed to show 10% progress.
  */
  long int k = 0, ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  long int j_start, j_end;

  double part_total = (double) (*partition_end - *partition_begin);

  long int _part_total, _temp_begin, _temp_end;

  _temp_begin = (*partition_begin_i+1)*(*partition_begin_i)/2;
  _temp_end = (*partition_end_i+1)*(*partition_end_i)/2;

  _temp_begin -= *partition_begin_i - *partition_begin_j + 1;
  _temp_end -= *partition_end_i - *partition_end_j;

  _part_total = _temp_end - _temp_begin;

  for(long int i = *partition_begin_i; i <= *partition_end_i; i++){
    if((i == *partition_begin_i) && (i == *partition_end_i)){
      j_start = *partition_begin_j;
      j_end = *partition_end_j;
    }
    else if(i == *partition_begin_i){
      j_start = *partition_begin_j;
      j_end = i-1;
    }
    else if(i == *partition_end_i){
      j_start = 0;
      j_end = *partition_end_j;
    }
    else{
      j_start = 0;
      j_end = i-1;
    }
    for(long int j = j_start; j <= j_end; j++){
      dist = get_distance_3d(x[i], y[i], z[i], x[j], y[j], z[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dd[ind] += 1.;
      }
      k += 1;
      if(k % (_part_total/10) == 0){
        cout << *prefix << "Auto Pairs = " << k/(_part_total/10) << '/' << 10 << endl;
      }
    }
  }
  for(int i = 0; i < numbins; i++){
    dd[i] *= 2.;
  }
}

void get_mpi_dr_3d(vector<float> &dr, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &x_data, vector<float> &y_data, vector<float> &z_data,
  vector<float> &x_rand, vector<float> &y_rand, vector<float> &z_rand,
  long int *partition_begin, long int *partition_end, string *prefix){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dr : vector
    Vector of cross pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  x_data, y_data, z_data : vector
    3D coordinate positions of the data.
  x_rand, y_rand, z_rand : vector
    3D coordinate positions of the randoms.
  partition_begin : long int
    Beginning of total partition count.
  partition_end : long int
    End of total partition count.
  prefix : string
    Prefix to be printed to show 10% progress.
  */
  long int ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  for(long int i = *partition_begin; i < *partition_end; i++){
    for(long int j = 0; j < x_rand.size(); j++){
      dist = get_distance_3d(x_data[i], y_data[i], z_data[i], x_rand[j], y_rand[j], z_rand[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dr[ind] += 1.;
      }
    }
    if((i - *partition_begin) % ((*partition_end - *partition_begin)/10) == 0){
      cout << *prefix << "Cross Pairs = " << (i - *partition_begin)/((*partition_end - *partition_begin)/10) + 1 << '/' << 10 << endl;
    }
  }
}

void get_mpi_dd_tomo(vector<float> &dd, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &phi, vector<float> &theta, long int *partition_begin, long int *partition_end, 
  long int *partition_begin_i, long int *partition_begin_j, long int *partition_end_i,
  long int *partition_end_j, string *prefix){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dd : vector
    Vector of auto pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  phi, theta : vector
    Coordinates on a sphere.
  partition_begin : long int
    Beginning of total partition count.
  partition_end : long int
    End of total partition count.
  partition_being_i, partition_begin_j : long int
    Index for rows and columns beginnings.
  partition_end_i, partition_end_j : long int
    Index for rows and columns endings.
  prefix : string
    Prefix to be printed to show 10% progress.
  */
  long int k = 0, ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  long int j_start, j_end;

  double part_total = (double) (*partition_end - *partition_begin);

  long int _part_total, _temp_begin, _temp_end;

  _temp_begin = (*partition_begin_i+1)*(*partition_begin_i)/2;
  _temp_end = (*partition_end_i+1)*(*partition_end_i)/2;

  _temp_begin -= *partition_begin_i - *partition_begin_j + 1;
  _temp_end -= *partition_end_i - *partition_end_j;

  _part_total = _temp_end - _temp_begin;

  for(long int i = *partition_begin_i; i <= *partition_end_i; i++){
    if((i == *partition_begin_i) && (i == *partition_end_i)){
      j_start = *partition_begin_j;
      j_end = *partition_end_j;
    }
    else if(i == *partition_begin_i){
      j_start = *partition_begin_j;
      j_end = i-1;
    }
    else if(i == *partition_end_i){
      j_start = 0;
      j_end = *partition_end_j;
    }
    else{
      j_start = 0;
      j_end = i-1;
    }
    for(long int j = j_start; j <= j_end; j++){
      dist = get_distance_tomo(phi[i], theta[i], phi[j], theta[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dd[ind] += 1.;
      }
      k += 1;
      if(k % (_part_total/10) == 0){
        cout << *prefix << "Auto Pairs = " << k/(_part_total/10) << '/' << 10 << endl;
      }
    }
  }
  for(int i = 0; i < numbins; i++){
    dd[i] *= 2.;
  }
}

void get_mpi_dr_tomo(vector<float> &dr, float minimum, float maximum, int numbins, bool uselog,
  vector<float> &phi_data, vector<float> &theta_data, vector<float> &phi_rand, vector<float> &theta_rand,
  long int *partition_begin, long int *partition_end, string *prefix){
  /* Function for parallelised auto pair distance counts. Only takes unique pairs, i.e. i, j when j < i.

  Parameters
  ----------
  dr : vector
    Vector of cross pair counts.
  minimum : float
    The minimum for the distance between pairs to be binned.
  maximum : float
    The maximum for the distance between pairs to be binned.
  numbins : int
    Number of bins for the data to be binned in.
  uselog : bool
    Determines whether to use log bins or not.
  phi_data, theta_data : vector
    Tomographic coordinate positions of the data.
  phi_rand, theta_rand : vector
    Tomographic coordinate positions of the randoms.
  partition_begin : long int
    Beginning of total partition count.
  partition_end : long int
    End of total partition count.
  prefix : string
    Prefix to be printed to show 10% progress.
  */
  long int ind;
  float dx, log10min, log10max, log10dx, dist;

  dx = (maximum - minimum)/((float)(numbins));
  log10min = log10(minimum);
  log10max = log10(maximum);
  log10dx = (log10max - log10min)/((float)(numbins));

  for(long int i = *partition_begin; i < *partition_end; i++){
    for(long int j = 0; j < phi_rand.size(); j++){
      dist = get_distance_tomo(phi_data[i], theta_data[i], phi_rand[j], theta_rand[j]);
      if (dist >= minimum and dist <= maximum){
        if(uselog == true){
          ind = floor((log10(dist) - log10min)/log10dx);
        }
        else{
          ind = floor((dist - minimum)/dx);
        }
        dr[ind] += 1.;
      }
    }
    if((i - *partition_begin) % ((*partition_end - *partition_begin)/10) == 0){
      cout << *prefix << "Cross Pairs = " << (i - *partition_begin)/((*partition_end - *partition_begin)/10) + 1 << '/' << 10 << endl;
    }
  }
}
