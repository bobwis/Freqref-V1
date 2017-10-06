namespace Simulation
{
    public class GPSSource : BaseSource
    {
        public GPSSource()
        {
            CurrentFreq = 10e6;
            AddJitter = true;
        }


        public WorldClock WorldClock { get; set; }
    }
}