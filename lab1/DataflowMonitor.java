import java.util.Iterator;
import java.util.ListIterator;
import java.util.LinkedList;
import java.util.BitSet;
import java.util.concurrent.*;
class Random {
	int	w;
	int	z;

	public Random(int seed)
	{
		w = seed + 1;
		z = seed * seed + seed + 2;
	}

	int nextInt()
	{
		z = 36969 * (z & 65535) + (z >> 16);
		w = 18000 * (w & 65535) + (w >> 16);

		return Math.abs((z << 16) + w);
	}
}

class Monitor {
	private LinkedList<Vertex> workList;
	private int working;
	private int nthread;

	public Monitor(LinkedList<Vertex> workList, int nthread){
		this.workList = workList;
		this.nthread = nthread;
	}

	public synchronized void addWork(Vertex v){
		workList.add(v);
		notifyAll();
	}

	public synchronized Vertex getWork(){
		working++;

		if(working == nthread){
				notifyAll();
				return null;
			}
		try {
			while(workList.isEmpty()){
				if(working == nthread)
					return null;
				wait();
			}
		}catch(InterruptedException e){
			System.out.println("whut , where the work at ? ");
		}
		working--;

		Vertex v = workList.remove();
		v.listed = false;
		if(v == null)
			System.out.println("WHAT THE FK");

		return v;
	}


}

class Task implements Runnable {
	private Monitor m;
	public Task(Monitor m){
		this.m = m;
	}
	public void run(){
		while(!Thread.currentThread().isInterrupted()){

			Vertex v = m.getWork();
			if( v != null ){

				v.computeIn(m);
			}else{
				break;
			}
		}
	}
}

class Vertex {
	int			index;
	volatile boolean			listed;
	LinkedList<Vertex>	pred;
	LinkedList<Vertex>	succ;
	volatile BitSet			in;
	volatile  BitSet			out;
	BitSet			use;
	BitSet			def;
	Vertex(int i)
	{
		index	= i;
		pred	= new LinkedList<Vertex>();
		succ	= new LinkedList<Vertex>();
		in	= new BitSet();
		out	= new BitSet();
		use	= new BitSet();
		def	= new BitSet();
	}

		public void computeIn(Monitor m){
		int			i;
		BitSet			old, newIn;
		Vertex			v;
		ListIterator<Vertex>	iter;

		iter = succ.listIterator();

		while (iter.hasNext()) {
			v = iter.next();
			out.or(v.in);
		}

		old = in;

		// in = use U (out - def)

		newIn = new BitSet();
		newIn.or(out);
		newIn.andNot(def);
		newIn.or(use);

		in = newIn;

		if (!in.equals(old)) {
			iter = pred.listIterator();

			while (iter.hasNext()) {
				v = iter.next();
				if (!v.listed) {
					m.addWork(v);
					v.listed = true;
				}
			}
		}
	}

	public void print(){
		int	i;

		System.out.print("use[" + index + "] = { ");
		for (i = 0; i < use.size(); ++i)
			if (use.get(i))
				System.out.print("" + i + " ");
		System.out.println("}");
		System.out.print("def[" + index + "] = { ");
		for (i = 0; i < def.size(); ++i)
			if (def.get(i))
				System.out.print("" + i + " ");
		System.out.println("}\n");

		System.out.print("in[" + index + "] = { ");
		for (i = 0; i < in.size(); ++i)
			if (in.get(i))
				System.out.print("" + i + " ");
		System.out.println("}");

		System.out.print("out[" + index + "] = { ");
		for (i = 0; i < out.size(); ++i)
			if (out.get(i))
				System.out.print("" + i + " ");
		System.out.println("}\n");
	}
}

class Dataflow {

	public static void connect(Vertex pred, Vertex succ)
	{
		pred.succ.addLast(succ);
		succ.pred.addLast(pred);
	}

	public static void generateCFG(Vertex vertex[], int maxsucc, Random r)
	{
		int	i;
		int	j;
		int	k;
		int	s;	// number of successors of a vertex.

		System.out.println("generating CFG...");

		connect(vertex[0], vertex[1]);
		connect(vertex[0], vertex[2]);

		for (i = 2; i < vertex.length; ++i) {
			s = (r.nextInt() % maxsucc) + 1;
			for (j = 0; j < s; ++j) {
				k = Math.abs(r.nextInt()) % vertex.length;
				connect(vertex[i], vertex[k]);
			}
		}
	}

	public static void generateUseDef(
		Vertex	vertex[],
		int	nsym,
		int	nactive,
		Random	r)
	{
		int	i;
		int	j;
		int	sym;

		System.out.println("generating usedefs...");

		for (i = 0; i < vertex.length; ++i) {
			for (j = 0; j < nactive; ++j) {
				sym = Math.abs(r.nextInt()) % nsym;

				if (j % 4 != 0) {
					if (!vertex[i].def.get(sym))
						vertex[i].use.set(sym);
				} else {
					if (!vertex[i].use.get(sym))
						vertex[i].def.set(sym);
				}
			}
		}
	}

	public static void liveness(Vertex vertex[], int nthread)
	{
		Vertex			u;
		Vertex			v;
		int			i;
		LinkedList<Vertex>	worklist;
		long			begin;
		long			end;

		System.out.println("computing liveness...");

		begin = System.nanoTime();
		worklist = new LinkedList<Vertex>();

		for (i = 0; i < vertex.length; ++i) {
			worklist.addLast(vertex[i]);
			vertex[i].listed = true;
		}


		Monitor m = new Monitor(worklist,nthread);
		LinkedList<Thread> ts = new LinkedList<Thread>();
		for (i = 0; i < nthread; i++){
			Thread t = new Thread(new Task(m));
			ts.add(t);
			t.start();
		}

		try{
			for(Thread t: ts)
				t.join();
		}
		catch(InterruptedException e){
			System.out.println("thread thread in liveness");
		}



		end = System.nanoTime();

		System.out.println("T = " + (end-begin)/1e9 + " s");
	}

	public static void main(String[] args)
	{
		int	i;
		int	nsym;
		int	nvertex;
		int	maxsucc;
		int	nactive;
		int	nthread;
		boolean	print;
		Vertex	vertex[];
		Random	r;

		r = new Random(1);

		nsym = Integer.parseInt(args[0]);
		nvertex = Integer.parseInt(args[1]);
		maxsucc = Integer.parseInt(args[2]);
		nactive = Integer.parseInt(args[3]);
		nthread = Integer.parseInt(args[4]);
		print = Integer.parseInt(args[5]) != 0;

		System.out.println("nsym = " + nsym);
		System.out.println("nvertex = " + nvertex);
		System.out.println("maxsucc = " + maxsucc);
		System.out.println("nactive = " + nactive);

		vertex = new Vertex[nvertex];

		for (i = 0; i < vertex.length; ++i)
			vertex[i] = new Vertex(i);

		generateCFG(vertex, maxsucc, r);
		generateUseDef(vertex, nsym, nactive, r);
		liveness(vertex, nthread);

		if (print)
			for (i = 0; i < vertex.length; ++i)
				vertex[i].print();
	}
}
