#! /usr/bin/env python

from matplotlib.pyplot import plot, show, figure, xlabel, ylabel, savefig
from numpy import array, vstack, max



def main():
    filename = 'scores_vs_dimensions.txt'

    dims = []
    for ii, line in enumerate(open(filename, 'r')):
        nums = [float(ss) for ss in line.split()]
        dims.append(int(nums[0]))
        row = nums[1:]
        if ii == 0:
            scores = array(row)
        else:
            scores = vstack((scores, row))

    dims = array(dims)


    differences = []
    for ii in range(scores.shape[0]):
        differences.append(scores[ii,0] - max(scores[ii,1:]))
    differences = array(differences)
        
    print dims
    print
    print scores
    print
    print differences

    #figure()
    #plot(dims)
    #figure()
    #plot(scores.T)
    figure()
    plot(dims, differences)
    plot(array([min(dims), max(dims)]), array([0, 0]), 'k:')
    xlabel('$\mathrm{Number\ of\ eigenvectors}$', fontsize=16)
    #ylabel('$\mathrm{Score_{correct}\;minus\;score_{best other}}$', fontsize=15)
    ylabel('$\mathrm{Correct\ class\ score\ minus\ highest\ other\ score}$', fontsize=16)
    savefig('scores_vs_dim.png')
    savefig('scores_vs_dim.pdf')

    show()


if __name__ == '__main__':
    main()
