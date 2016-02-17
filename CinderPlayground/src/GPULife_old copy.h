 //
//  CinderPlaygroundApp.h
//  CinderPlayground
//
//  Created by Ilya Solovyov on 11/12/15.
//
//

#ifndef GPULife_h
#define GPULife_h

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Utilities.h"


class CinderPlaygroundApp : public ci::app::App
{
public:
    ~CinderPlaygroundApp();
    
    void setup() override;
    void mouseMove( cinder::app::MouseEvent event ) override;
    void mouseDown( cinder::app::MouseEvent event ) override;
    void mouseUp( cinder::app::MouseEvent event ) override;
    void update() override;
    void draw() override;
    
protected:
    struct CellHelper
    {
    private:
        void _init(int x, int y)
        {
            this->x = x;
            this->y = y;
            neighbors = NULL;
            neighborsClasses = NULL;
        }
        
    public:
        int x;
        int y;
        CellHelper** neighbors;
        int* neighborsClasses;
        
        CellHelper()
        {
            _init(0, 0);
        }
        CellHelper(int x, int y)
        {
            _init(x, y);
        }
        ~CellHelper()
        {
            if (this->neighbors != NULL)
            {
                delete [] this->neighbors;
                this->neighbors = NULL;
            }
            if (this->neighborsClasses != NULL)
            {
                delete [] this->neighborsClasses;
                this->neighborsClasses = NULL;
            }
        }
        
        static int cycledIndex(int index, int length, bool* cycled)
        {
            if (index < 0)
            {
                *cycled = true;
                return cycledIndex(length + index, length, cycled);
            }
            else if (index >= length)
            {
                *cycled = true;
                return cycledIndex(index - length, length, cycled);
            }
            else
                return index;
        }
        
        void FillNeighbors(CellHelper** grid, int width, int height, int ruleRadius)
        {
            int diameter = 2 * ruleRadius + 1;
            int neigborsCount = diameter * diameter - 1;
            this->neighbors = new CellHelper*[neigborsCount];
            this->neighborsClasses = new int[neigborsCount];
            
            int counter = 0;
            for (int k = -ruleRadius; k <= ruleRadius; ++k)
            {
                for (int l = -ruleRadius; l <= ruleRadius; l++)
                {
                    if (k != 0 || l != 0)
                    {
                        bool cycledI = false;
                        bool cycledJ = false;
                        
                        this->neighbors[counter] = &grid[cycledIndex(x + k, width, &cycledI)][cycledIndex(y + l, height, &cycledJ)];
                        this->neighborsClasses[counter] = 0;
                        if (!cycledI && cycledJ)
                            this->neighborsClasses[counter] = 1;
                        else if (cycledI && !cycledJ)
                            this->neighborsClasses[counter] = 2;
                        else if (cycledI && cycledJ)
                            this->neighborsClasses[counter] = 3;
                        
                        counter++;
                    }
                }
            }
        }
    };
    GLuint _vboCells;
    GLuint _uboCells;
    GLuint _vboMask;
    
    GLuint _feedbackVao;
    GLuint _feedbackTfbo;
    GLuint _feedbackShader;
    GLuint _feedbackProgram;
    
    GLfloat* _dataInputBuffer;
    GLfloat* _dataFeedbackBuffer;
    int _dataLength;
    int _dataBytesSize;
    int _gridWidth;
    int _gridHeight;
    int _ruleRadius;
    int _neigborsCount;
    
    void _prepareFeedback();
    void _prepareFeedbackProgram();
    void _prepareFeedbackBuffers();
    void _prepareFeedbackVertexArray();
    
    void _prepareNextUpdate();
    void _updateFeedback();
};

#endif /* GPULife_h */
