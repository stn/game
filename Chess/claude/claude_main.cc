#include "claude.h"

#include <iostream>

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace {
  Position positions[100];
};  // namespace positions;


// Benchmark
namespace {
  
  double GetTime() {
    struct timeval t;
    struct timezone tzp;
    gettimeofday(&t, &tzp);
    return t.tv_sec + t.tv_usec*1e-6;
  }
  
  void Benchmark() {
    /*
     double s0 = GetTime();
     Move* m = new Move[10000000];
     double e0 = GetTime();
     printf("time0=%f\n", e0 - s0);
     delete m;
     */
    
    srand(1);
    double start = GetTime();
    
    //RandomPlayer player;
    MinMaxPlayer player(2);
    
    for (int i = 0; i < 100; ++i) {
      positions[0].StartPosition();
      int ply = 0;
      while (1) {
        Position& pos = positions[ply];
        pos.CalcMoves();
        //printf("%d.\n", ply);
        //pos.Print();
        Move move;
        if (!player.NextMove(pos, &move)) {
          break;
        }
        //printf("-> ");
        //move.Print();
        //printf("\n\n");
        pos.DoMove(move, &positions[ply+1]);
        ++ply;
        if (ply >= 99) {
          break;
        }
      }
    }
    
    double end = GetTime();
    cout << "time=" << end - start << endl;
  }
  
}  // namespace

int main(int argc, char* argv[]) {
  while (*++argv) {
    if (**argv == '-') {
      switch ((*argv)[1]) {
        case 'B': 
          Benchmark();
          return 0;
        default:
          cerr << "Unkown option " << *argv << endl;
          // nothing
      } 
    } else {
      break; 
    } 
  }
  
  srand((unsigned)time(NULL));
  
  positions[0].StartPosition();
  
  //RandomPlayer player;
  MinMaxPlayer player(4);
  
  int ply = 0;
  while (1) {
    Position& pos = positions[ply];
    pos.CalcMoves();
    cout << ply << "." << endl;
    pos.Print();
    if (pos.next_moves().empty()) {
      if (pos.IsCheck()) {
        // check mate!
        if (ply % 2 == 0) {
          cout << "0-1" << endl;
        } else {
          cout << "1-0" << endl;
        }
      } else {
        // Stale mate
        cout << "1/2-1/2" << endl;
      }
      return 0;
    }
    if (ply % 2 == 0) {
      // player move
      while (1) {
        Move move = Move::Read();
        if (pos.IsValidMove(move)) {
          cout << endl;
          pos.DoMove(move, &positions[ply+1]);
          break;
        }
      }
    } else {
      Move move;
      double start = GetTime();
      player.NextMove(pos, &move);
      double end = GetTime();
      cout << "time = " << end - start << endl;
      cout << "time/count = " << (end - start)/player.count * 1000. << " ms" << endl;
      cout << "score = " << player.last_score << endl;
      cout << "-> ";
      move.Print();
      cout << endl << endl;
      pos.DoMove(move, &positions[ply+1]);
    }
    ++ply;
  }
  
  return 0;
}