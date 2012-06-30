#include "claude.h"

#include <boost/algorithm/string/predicate.hpp>
#include <fstream>
#include <iostream>
#include <string>

#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

namespace {
  Position positions[100];
};  // namespace positions;

int main(int argc, char* argv[]) {
  while (*++argv) {
    if (**argv == '-') {
      switch ((*argv)[1]) {
        case 'h': 
          cout << "Usage: uci" << endl;
          return 0;
        default:
          printf("Unkown option %s\n", *argv);
          // nothing
      } 
    } else {
      break; 
    } 
  }
  
  srand((unsigned)time(NULL));
  
  ofstream log("/tmp/uci.log");
  
  log << "START" << endl;
  
  string line;
  while (getline(cin, line)) {
    log << "IN: " << line << endl;
    if (line == "uci") {
      cout << "id name Claude 0.1" << endl;
      cout << "id author Akira Ishino" << endl;
      cout << "uciok" << endl;
      cout.flush();
    } else if (line == "isready") {
      cout << "readyok" << endl;
    } else if (boost::starts_with(line, "position ")) {
      log << "POSITION" << endl;
      positions[0].StartPosition();
    } else if (line == "go infinit") {
      //RandomPlayer player;
      MinMaxPlayer player(4);
      
      int ply = 0;
      while (1) {
        Position& pos = positions[ply];
        pos.CalcMoves();
        printf("%d.\n", ply);
        pos.Print();
        if (pos.next_moves().empty()) {
          if (pos.IsCheck()) {
            // check mate!
            if (ply % 2 == 0) {
              printf("0-1\n");
            } else {
              printf("1-0\n");
            }
          } else {
            // Stale mate
            printf("1/2-1/2\n");
          }
          return 0;
        }
        if (ply % 2 == 0) {
          // player move
          while (1) {
            Move move = Move::Read();
            if (pos.IsValidMove(move)) {
              printf("\n");
              pos.DoMove(move, &positions[ply+1]);
              break;
            }
          }
        } else {
          Move move;
          player.NextMove(pos, &move);
          printf("score = %d\n", player.last_score);
          printf("-> ");
          move.Print();
          printf("\n\n");
          pos.DoMove(move, &positions[ply+1]);
        }
        ++ply;
      }
      
    }
  }
  
  return 0;
}