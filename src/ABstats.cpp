/*
   DDS, a bridge double dummy solver.

   Copyright (C) 2006-2014 by Bo Haglund /
   2014-2018 by Bo Haglund & Soren Hein.

   See LICENSE and README.
*/

/*
   ABstats is a simple object for AB statistics and return values.
*/


#include "dds.h"
#include "ABstats.h"


ABstats::ABstats()
{
  fname = "";
  fileSet = false;
  fp = stdout;

  for (int p = 0; p < DDS_AB_POS; p++)
    sprintf(name[p], "Position %4d", p);

  ABstats::Reset();
}


ABstats::~ABstats()
{
  // Nothing to do
  if (fp != stdout && fp != nullptr)
    fclose(fp);
}


void ABstats::Reset()
{
  for (int p = 0; p < DDS_AB_POS; p++)
  {
    sumNew[p] = 0;
    psumNew[p] = 0;

    for (int depth = 0; depth < DDS_MAXDEPTH; depth++)
      counter[p][depth] = 0;
  }

  for (int side = 0; side < 2; side++)
  {
    for (int depth = 0; depth < DDS_MAXDEPTH; depth++)
      score[side][depth] = 0;
  }

  for (int depth = 0; depth < DDS_MAXDEPTH; depth++)
    nodes[depth] = 0;

  allnodes = 0;

  for (int side = 0; side < 2; side++)
  {
    for (int depth = 0; depth < DDS_MAXDEPTH; depth++)
      ABscores[side].list[depth] = 0;

    ABscores[side].sum = 0;
    ABscores[side].sumWeighted = 0;

  }
}


void ABstats::ResetCum()
{
  for (int depth = 0; depth < DDS_MAXDEPTH; depth++)
    nodesCum[depth] = 0;

  allnodesCum = 0;

  scoreCum[1] = 0;
  scoreCum[0] = 0;

  pscoreCum[1] = 0;
  pscoreCum[0] = 0;

  for (int p = 0; p < DDS_AB_POS; p++)
  {
    counterCum[p] = 0;
    pcounterCum[p] = 0;
  }

  for (int side = 0; side < 2; side++)
  {
    ABscores[side].sumCum = 0;
    ABscores[side].sumCumWeighted = 0;
  }
}


void ABstats::SetFile(const string& fnameIn)
{
  fname = fnameIn;
}


void ABstats::SetName(int no, char * ourName)
{
  if (no < 0 || no >= DDS_AB_POS)
    return;

  sprintf(name[no], "%s", ourName);
}


void ABstats::IncrPos(
  int no, 
  bool side, 
  int depth)
{
  if (no < 0 || no >= DDS_AB_POS)
    return;

  counter[no][depth]++;
  sumNew[no]++;
  psumNew[no] += depth;

  const int iside = (side ? 1 : 0);

  score[iside][depth]++;

  ABscores[iside].list[depth]++;
  ABscores[iside].sum++;
  ABscores[iside].sumWeighted += depth;
  ABscores[iside].sumCum++;
  ABscores[iside].sumCumWeighted += depth;

}


void ABstats::IncrNode(int depth)
{
  nodes[depth]++;
  nodesCum[depth]++;
  allnodes++;
}


int ABstats::GetNodes() const
{
  return allnodes;
}


#include "../include/portab.h"
void ABstats::PrintStatsPosition(FILE * fpl) const
{
  UNUSED(fpl);
}


void ABstats::PrintStats()
{
  if (! fileSet)
  {
    if (fname != "")
    {
      fp = fopen(fname.c_str(), "w");
      if (! fp)
        fp = stdout;
    }
    fileSet = true;
  }


  int sumScore1 = 0 , sumScore0 = 0;
  int psumScore1 = 0 , psumScore0 = 0;

  for (int d = 0; d < DDS_MAXDEPTH; d++)
  {
    sumScore1 += score[1][d]; // ABscores[1].sum
    sumScore0 += score[0][d];

    psumScore1 += d * score[1][d]; // ABscores[1].sumWeighted;
    psumScore0 += d * score[0][d];
  }

  allnodesCum += allnodes;

  scoreCum[1] += sumScore1; // ABscores[1].sumCum;
  scoreCum[0] += sumScore0;

  pscoreCum[1] += psumScore1; // ABscores[1].sumCumWeighted
  pscoreCum[0] += psumScore0;

  for (int p = 0; p < DDS_AB_POS; p++)
  {
    counterCum[p] += sumNew[p];
    pcounterCum[p] += psumNew[p];
  }

if (ABscores[1].sum != sumScore1)
  fprintf(fp, "1sum %d %d\n", ABscores[1].sum, sumScore1);

if (ABscores[0].sum != sumScore0)
  fprintf(fp, "0sum %d %d\n", ABscores[0].sum, sumScore0);

if (ABscores[1].sumWeighted != psumScore1)
  fprintf(fp, "1sumw %d %d\n", ABscores[1].sumWeighted, psumScore1);

if (ABscores[0].sumWeighted != psumScore0)
  fprintf(fp, "0sumw %d %d\n", ABscores[0].sumWeighted, psumScore0);

if (ABscores[1].sumCum != scoreCum[1])
  fprintf(fp, "1sumc %d %d\n", ABscores[1].sumCum, scoreCum[1]);

if (ABscores[0].sumCum != scoreCum[0])
  fprintf(fp, "0sumc %d %d\n", ABscores[0].sumCum, scoreCum[0]);

if (ABscores[1].sumCumWeighted != pscoreCum[1])
  fprintf(fp, "1sumcw %d %d\n", ABscores[1].sumCumWeighted, pscoreCum[1]);

if (ABscores[0].sumCumWeighted != pscoreCum[0])
  fprintf(fp, "0sumcw %d %d\n", ABscores[0].sumCumWeighted, pscoreCum[0]);


  int s = sumScore1 + sumScore0;
  int cs = scoreCum[1] + scoreCum[0];
  if (s)
  {
    fprintf(fp, "%2s %-20s %8s %5s %5s %8s %5s %5s\n",
            "No",
            "Return",
            "Count",
            "%",
            "d_avg",
            "Cumul",
            "%",
            "d_avg");

    fprintf(fp, "-----------------------------------");
    fprintf(fp, "------------------------------\n");

    fprintf(fp, "%2s %-20s %8d %5.1f %5.1f %8d %5.1f %5.1f\n",
            "",
            "Side1",
            sumScore1,
            100. * sumScore1 / static_cast<double>(s),
            psumScore1 / static_cast<double>(s),
            scoreCum[1],
            100. * scoreCum[1] / static_cast<double>(cs),
            pscoreCum[1] / static_cast<double>(cs));

    fprintf(fp, "%2s %-20s %8d %5.1f %5.1f %8d %5.1f %5.1f\n\n",
            "",
            "Side0",
            sumScore0,
            100. * sumScore0 / static_cast<double>(s),
            psumScore0 / static_cast<double>(s),
            scoreCum[0],
            100. * scoreCum[0] / static_cast<double>(cs),
            pscoreCum[0] / static_cast<double>(cs));

    for (int p = 0; p < DDS_AB_POS; p++)
    {
      if (sumNew[p])
      {
        fprintf(fp, "%2d %-20s %8d %5.1f %5.1f %8d %5.1f %5.1f\n",
                p,
                name[p],
                sumNew[p],
                100. * sumNew[p] / static_cast<double>(s),
                psumNew[p] / static_cast<double>(sumNew[p]),
                counterCum[p],
                100. * counterCum[p] / static_cast<double>(cs),
                pcounterCum[p] / static_cast<double>(counterCum[p]));
      }
      else if (counterCum[p])
      {
        fprintf(fp, "%2d %-20s %8d %5.1f %5s %8d %5.1f %5.1f\n",
                p,
                name[p],
                sumNew[p],
                100. * sumNew[p] / static_cast<double>(s),
                "",
                counterCum[p],
                100. * counterCum[p] / static_cast<double>(cs),
                pcounterCum[p] / static_cast<double>(counterCum[p]));
      }
    }
  }

  fprintf(fp, "\n%5s %6s %6s %5s %5s %6s\n",
          "Depth",
          "Nodes",
          "Cumul",
          "Cum%",
          "Cumc%",
          "Branch");

  fprintf(fp, "------------------------------------------\n");

  int c = 0;
  double np = 0., ncp = 0.;
  for (int d = DDS_MAXDEPTH - 1; d >= 0; d--)
  {
    if (nodesCum[d] == 0)
      continue;

    c += nodesCum[d];
    np += d * nodes[d];
    ncp += d * nodesCum[d];

    fprintf(fp, "%5d %6d %6d %5.1f %5.1f",
            d,
            nodes[d],
            nodesCum[d],
            100. * nodesCum[d] / static_cast<double>(allnodesCum),
            100. * c / static_cast<double>(allnodesCum));

    // "Branching factor" from end of one trick to end of
    // the previous trick.
    if ((d % 4 == 1) &&
        (d < DDS_MAXDEPTH - 4) &&
        (nodesCum[d + 4] > 0))
      fprintf(fp, " %5.2f",
              nodesCum[d] / static_cast<double>(nodesCum[d + 4]));
    fprintf(fp, "\n");
  }

  fprintf(fp, "\n%-5s %6d %6d\n",
          "Total", allnodes, allnodesCum);

  if (allnodes)
  {
    fprintf(fp, "%-5s %6.1f %6.1f\n",
            "d_avg",
            np / static_cast<double>(allnodes),
            ncp / static_cast<double>(allnodesCum));
  }
  else if (allnodesCum)
  {
    fprintf(fp, "\n%-5s %6s %6.1f\n",
            "Avg",
            "-",
            ncp / static_cast<double>(allnodesCum));
  }

  fprintf(fp, "%-5s %6d\n\n\n", "Diff",
          allnodes - sumScore1 - sumScore0);

#ifdef DDS_AB_DETAILS
  fprintf(fp, "%2s %6s %6s",
          "d",
          "Side1",
          "Side0");

  for (int p = 0; p < DDS_AB_POS; p++)
    fprintf(fp, " %5d", p);
  fprintf(fp, "\n------------------------------");
  fprintf(fp, "-----------------------------\n");


  for (int d = DDS_MAXDEPTH - 1; d >= 0; d--)
  {
    if (score[1][d] == 0 && score[0][d] == 0)
      continue;

    fprintf(fp, "%2d %6d %6d",
            d, score[1][d], score[0][d]);

    for (int p = 0; p < DDS_AB_POS; p++)
      fprintf(fp, " %5d", counter[p][d]);
    fprintf(fp, "\n");
  }

  fprintf(fp, "--------------------------------");
  fprintf(fp, "---------------------------\n");

  fprintf(fp, "%2s %6d %6d",
          "S", sumScore1, sumScore0);

  for (int p = 0; p < DDS_AB_POS; p++)
    fprintf(fp, " %5d", sumNew[p]);
  fprintf(fp, "\n\n");
#endif

}
