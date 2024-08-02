
///
//////
/////////// include
//////
///

#include <iostream>
#include <vector>

///
//////
/////////// namespace
//////
///

using namespace std;

///
//////
/////////// macro: generic
//////
///

#define DBG(...) { \
    cout << "DBG: "; \
    cout << __VA_ARGS__; \
    cout << endl; \
}

///
//////
/////////// macro: icon
//////
///

#define ICON_PAWN_WHITE   "♙"
#define ICON_KNIGHT_WHITE "♘"
#define ICON_BISHOP_WHITE "♗"
#define ICON_ROOK_WHITE   "♖"
#define ICON_QUEEN_WHITE  "♕"
#define ICON_KING_WHITE   "♔"

#define ICON_PAWN_BLACK   "♟︎"
#define ICON_KNIGHT_BLACK "♞"
#define ICON_BISHOP_BLACK "♝"
#define ICON_ROOK_BLACK   "♜"
#define ICON_QUEEN_BLACK  "♛"
#define ICON_KING_BLACK   "♚"

#define ICON_WARNING "⚠"

///
//////
/////////// class: piece
//////
///

class Piece{

    public:

        string icon = ICON_WARNING;

        int forward_y = 0;
        // int forward_x = 0; // this only makes sense in 4way chess

};

///
//////
/////////// class: tile
//////
///

class Tile{

    public:

        Piece * piece = nullptr;

        // horizontal/vertical
        Tile * neighbour_up = nullptr;
        Tile * neighbour_down = nullptr;
        Tile * neighbour_left = nullptr;
        Tile * neighbour_right = nullptr;
        // diagonal
        Tile * neighbour_upleft = nullptr;
        Tile * neighbour_upright = nullptr;
        Tile * neighbour_downleft = nullptr;
        Tile * neighbour_downright = nullptr;

        int y;
        int x;

};

///
//////
/////////// class: board
//////
///

class Board{

    public:

        vector<Tile *> tiles = {};

        Tile * get_tile_at(int y, int x){

            size_t idx = y * 8 + x;

            if(idx >= tiles.size()){
                return nullptr;
            }

            return tiles[idx];

        }

};

///
//////
/////////// function: main
//////
///

int main(){

    Board * board = new Board;

    for(int y=0; y<8; ++y){
        for(int x=0; x<8; ++x){

            Tile * tile = new Tile{
                .y = y,
                .x = x,
            };

            board->tiles.push_back(tile);

        }
    }

    // connect neighbours
    
    for(Tile * tile : board->tiles){

        { // up
            int nei_up_y = tile->y - 1;
            int nei_up_x = tile->x;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_up = nei;
            }
        }

        { // down
            int nei_up_y = tile->y + 1;
            int nei_up_x = tile->x;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_down = nei;
            }
        }

        { // left
            int nei_up_y = tile->y;
            int nei_up_x = tile->x - 1;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_left = nei;
            }
        }

        { // right
            int nei_up_y = tile->y;
            int nei_up_x = tile->x + 1;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_right = nei;
            }
        }

        { // up left
            int nei_up_y = tile->y - 1;
            int nei_up_x = tile->x - 1;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_upleft = nei;
            }
        }

        { // up right
            int nei_up_y = tile->y - 1;
            int nei_up_x = tile->x + 1;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_upright = nei;
            }
        }

        { // down left
            int nei_up_y = tile->y + 1;
            int nei_up_x = tile->x - 1;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_downleft = nei;
            }
        }

        { // down right
            int nei_up_y = tile->y + 1;
            int nei_up_x = tile->x + 1;
            Tile * nei = board->get_tile_at(nei_up_y, nei_up_x);
            if(nei){
                tile->neighbour_downright = nei;
            }
        }

    }

    // Piece * pawn_black_0 = new Piece{
    //     .icon = ICON_PAWN_BLACK,
    //     .forward_y = -1,
    // };

    // DBG(ICON_PAWN_BLACK);

    // DBG(pawn_black_0->icon);
    // DBG(pawn_black_0->forward_y);

    return 0;
}
