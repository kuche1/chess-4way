
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

#define ERR(...) { \
    cout << "ERROR:" << endl; \
    cout << __VA_ARGS__ << endl; \
    cout << "(" << endl; \
    cout << "    location:" << endl; \
    cout << "        file `" << __FILE__ << "`" << endl; \
    cout << "        line " << __LINE__ << endl; \
    cout << "        function `" << __FUNCTION__ << "`" << endl; \
    cout << "        commit id `" << COMMIT_ID << "`" << endl; \
    cout << ")" << endl; \
    cout << endl; \
    exit(1); \
}

///
//////
/////////// define: icon
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
/////////// module: terminal escape code
//////
///

void disp_clear(){
    cout << "\033[H\033[J";
}

void disp_cur_set(int y, int x){
    cout << "\033[";
    cout << y + 1;
    cout << ";";
    cout << x + 1;
    cout << "H";
}

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

        int y;
        int x;

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

        Piece * piece = nullptr;

};

///
//////
/////////// class: board
//////
///

class Board{

    public:

        vector<Tile *> tiles = {};

        Board(){

            if(this->tiles.size() != 0){
                ERR("already initialised");
            }

            for(int y=0; y<8; ++y){
                for(int x=0; x<8; ++x){

                    Tile * tile = new Tile{
                        .y = y,
                        .x = x,
                    };

                    this->tiles.push_back(tile);

                }
            }

        }

        ~Board(){

            for(Tile * tile : this->tiles){

                delete tile;

            }

        }

        void connect_neighbours(){

            for(Tile * tile : this->tiles){

                { // up
                    int nei_up_y = tile->y - 1;
                    int nei_up_x = tile->x;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_up = nei;
                    }
                }

                { // down
                    int nei_up_y = tile->y + 1;
                    int nei_up_x = tile->x;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_down = nei;
                    }
                }

                { // left
                    int nei_up_y = tile->y;
                    int nei_up_x = tile->x - 1;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_left = nei;
                    }
                }

                { // right
                    int nei_up_y = tile->y;
                    int nei_up_x = tile->x + 1;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_right = nei;
                    }
                }

                { // up left
                    int nei_up_y = tile->y - 1;
                    int nei_up_x = tile->x - 1;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_upleft = nei;
                    }
                }

                { // up right
                    int nei_up_y = tile->y - 1;
                    int nei_up_x = tile->x + 1;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_upright = nei;
                    }
                }

                { // down left
                    int nei_up_y = tile->y + 1;
                    int nei_up_x = tile->x - 1;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_downleft = nei;
                    }
                }

                { // down right
                    int nei_up_y = tile->y + 1;
                    int nei_up_x = tile->x + 1;
                    Tile * nei = this->get_tile_at(nei_up_y, nei_up_x);
                    if(nei){
                        tile->neighbour_downright = nei;
                    }
                }

            }

        }

        Tile * get_tile_at(int y, int x){

            auto [fail_ci, idx] = this->calc_idx(y, x);
            if(fail_ci){
                return nullptr;
            }

            return tiles[idx];

        }

        void set_tile_at(int y, int x, Tile * tile){

            auto [fail_ci, idx] = this->calc_idx(y, x);
            if(fail_ci){
                ERR("invalid tile y=" << y << " x=" << x);
            }

            if(tiles[idx]){
                ERR("tile not empty y=" << y << " x=" << x);
            }

            tiles[idx] = tile;

        }
    
    private:

        pair<bool, ssize_t> calc_idx(int y, int x){
            if(y < 0 || y >= 8 || x < 0 || x >= 8){
                return {true, 0};
            }
            return {false, y * 8 + x};
        }

};

///
//////
/////////// function: main
//////
///

int main(){

    Board * board = new Board;
    
    board->connect_neighbours();

    // ...

    disp_clear();

    for(Tile * tile : board->tiles){

        Piece * piece = tile->piece;
        if(!piece){
            continue;
        }

        disp_cur_set(tile->y, tile->x);
        
        cout << piece->icon;
    }

    // Piece * pawn_black_0 = new Piece{
    //     .icon = ICON_PAWN_BLACK,
    //     .forward_y = -1,
    // };

    // DBG(ICON_PAWN_BLACK);

    // DBG(pawn_black_0->icon);
    // DBG(pawn_black_0->forward_y);

    delete board;

    return 0;
}
