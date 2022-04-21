#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>

#define MAX(a, b) ((a > b) ? (a) : (b))
#define ABS(a) ((a < 0) ? (-a) : (a))

double pWin(double ***p, int max, int i, int j, int k)
{
    if (i + k >= max)
        return 1.0;
    if (j >= max)
        return 0.0;
    return p[i][j][k];
}

void updateSingleEstimate(double ***p, int die, int max, int i, int j, int k)
{
    // From the corresponding state for the other player after ending turn, the compliment of their chance to win from that state
    double pHold = 1.0 - pWin(p, max, j, i + k, 0);

    //     1/die chance of losing the points and 1/die each chance of incrementing turn total (k) by [2..die]
    double pRoll = (1.0 - pWin(p, max, j, i, 0));
    for (int d = 2; d <= die; d++)
    {
        pRoll += pWin(p, max, i, j, k + d);
    }
    pRoll /= (double)die;

    p[i][j][k] = MAX(pHold, pRoll);
}

void iterateLayer(double ***p, int die, int max, int sumIJ, double eps)
{
    double oldP;
    double change;
    double maxChange;
    
    do
    {
        maxChange = 0;
        for (int i = max - 1; i >= 0; i--)
        {
            int j = sumIJ - i;

            if (i < max && j < max && j >= 0)
            {
                for (int k = 0; k < max - i; k++)
                {
                    oldP = p[i][j][k];
                    updateSingleEstimate(p, die, max, i, j, k);
                    change = ABS(p[i][j][k] - oldP);
                    maxChange = MAX(change, maxChange);
                }
            }
        }
    } while (maxChange >= eps);
}

void initMem(int ****roll, double ****p, int max)
{
    *roll = malloc(max * sizeof(int **));
    *p = malloc(max * sizeof(double **));
    for (int i = 0; i < max; i++)
    {
        (*roll)[i] = malloc(max * sizeof(int *));
        (*p)[i] = malloc(max * sizeof(double *));
        for (int j = 0; j < max; j++)
        {
            (*roll)[i][j] = malloc((max - i) * sizeof(int));
            (*p)[i][j] = malloc((max - i) * sizeof(double));
            for (int k = 0; k < max - i; k++)
            {
                (*roll)[i][j][k] = 0;
                (*p)[i][j][k] = 0;
            }
        }
    }
}

void freeMem(int ***roll, double ***p, int max)
{
    for (int i = 0; i < max; i++)
    {
        for (int j = 0; j < max; j++)
        {
            free(roll[i][j]);
            free(p[i][j]);
        }
        free(roll[i]);
        free(p[i]);
    }
    free(roll);
    free(p);
}

void solve(int ***roll, double ***p, int die, int max, double eps)
{
    for (int s = max * 2 - 2; s >= 0; s--)
    {
        // the sum of the two plyers' banked points can never decrease,
        // this can be used to iteratively work down from the highest sum to the least sum
        // which allows smaller clusters of value iteration at a time, and thus
        // more rapid convergence.
        iterateLayer(p, die, max, s, eps);
    }
    // p array now holds the probability of winning at each reachable non-terminal state
    // next use it to compute roll or not array

    for (int i = 0; i < max; i++)
    {
        for (int j = 0; j < max; j++)
        {
            for (int k = 0; k < max - i; k++)
            {
                double pHold = 1.0 - pWin(p, max, j, i + k, 0);
                double pRoll = (1.0 - pWin(p, max, j, i, 0));
                for (int d = 2; d <= die; d++)
                {
                    pRoll += pWin(p, max, i, j, k + d);
                }
                pRoll /= (double)die;

                roll[i][j][k] = (pRoll > pHold);
            }
        }
    }
    // By the end, roll array now contains whether or not an optimal player should roll at each state.
}

int myAtoi(const char *str)
{
    int sign = 1, base = 0, i = 0;

    while (str[i] == ' ')
        i++;

    if (str[i] == '-' || str[i] == '+')
    {
        sign = 1 - 2 * (str[i++] == '-');
    }

    while (str[i] >= '0' && str[i] <= '9')
    {
        if (base > INT_MAX / 10 || (base == INT_MAX / 10 && str[i] - '0' > 7))
        {
            if (sign == 1)
                return INT_MAX;
            else
                return INT_MIN;
        }

        base = 10 * base + (str[i++] - '0');
    }

    return base * sign;
}

void solveNew(int *dieSize, int *maxScore, int ****roll, double ****p, double eps)
{
    char input[100];
    char *token;

    // avoid mem leak
    if (roll != NULL) freeMem(*roll, *p, *maxScore);

    *dieSize = 0;
    *maxScore = 0;

    while (*dieSize <= 1)
    {
        printf("Die size (integer >= 2): ");
        fgets(input, 100, stdin);
        token = strtok(input, " \n");
        *dieSize = myAtoi(token);
    }

    while (*maxScore <= 1)
    {
        printf("Maximum score (integer >= 2): ");
        fgets(input, 100, stdin);
        token = strtok(input, " \n");
        *maxScore = myAtoi(token);
    }

    printf("Calculating optimal play matrix...\n");
    initMem(roll, p, *maxScore);
    solve(*roll, *p, *dieSize, *maxScore, eps);
    printf("Done!\n");
}

void query(int ***roll, double ***p, int max, int i, int j, int k)
{
    int rollOrNot;
    double probWin = pWin(p, max, i, j, k);
    rollOrNot = (probWin == 1.0 || probWin == 0.0) ? 0 : roll[i][j][k];
    printf("\nMy score = %d\nOpponent's Score = %d\nTurn total = %d\
            \nprobability of winning = %.4lf%%\nOptimal action = %s\
            \n", i, j, k, 100 * probWin, ((rollOrNot) ? "Roll" : "Hold"));
}

int rollDie(int d)
{
    return 1 + rand() % d;
}

void saveToCSV(FILE *fp, int ***roll, double ***p, int maxScore)
{
    printf("saving...\n");
    fprintf(fp, "i,j,k,roll,p\n");
    for (int i = 0; i < maxScore; i++)
    {
        for (int j = 0; j < maxScore; j++) {
            for (int k = 0; k < maxScore - i; k++) {
                fprintf(fp, "%d,%d,%d,%d,%lf\n", i, j, k, roll[i][j][k], p[i][j][k]);
            }
        }
    }
    printf("Done!\n");
}

int main(int argc, char *argv[])
{
    char input[100];
    char *token;
    int dieSize = 0, maxScore = 0;
    double eps = 0.0000001;
    int ***roll = NULL;
    double ***p = NULL;

    srand(time(0));

    printf("Welcome to Pig!\n");

    while (1)
    {
        printf("\n");
        fgets(input, 100, stdin);
        token = strtok(input, " \n");
        if (token == NULL)
            continue;
        else if (!strcmp(token, "q"))
            break;
        else if (!strcmp(token, "new"))
            solveNew(&dieSize, &maxScore, &roll, &p, eps);
        else if (!strcmp(token, "query"))
        {
            if (roll == NULL)
            {
                printf("you don't have a roll matrix yet, call new to solve a game of pig first.\n");
                continue;
            }
            token = strtok(NULL, " \n");
            if (token == NULL)
                goto badInputQuery;
            int i = myAtoi(token);
            token = strtok(NULL, " \n");
            if (token == NULL)
                goto badInputQuery;
            int j = myAtoi(token);
            token = strtok(NULL, " \n");
            if (token == NULL)
                goto badInputQuery;
            int k = myAtoi(token);
            query(roll, p, maxScore, i, j, k);
            continue;
        badInputQuery:
            printf("need 3 parameters: <own banked> <other banked> <round total>\n");
        }
        else if (!strcmp(token, "roll"))
        {
            token = strtok(NULL, " \n");
            if (token == NULL)
                goto badInputRoll;
            int d = myAtoi(token);
            d = rollDie(d);
            printf("Result: %d\n", d);
            continue;
        badInputRoll:
            printf("need 1 parameter: <# of die faces>\n");
        } else if (!strcmp(token, "save")) {
            FILE *fp = fopen("savedsolution.csv", "w");
            saveToCSV(fp, roll, p, maxScore);
            fclose(fp);
        }
    }

    freeMem(roll, p, maxScore);
    return 0;
}