import numpy
import gillespy2
import pickle
import sys
import subprocess
import hashlib

class Dimerization(gillespy2.Model):
    def __init__(self, parameter_values=None):
        # First call the gillespy2.Model initializer.
        super().__init__(self)

        # Define parameters for the rates of creation and dissociation.
        k_c = gillespy2.Parameter(name='k_c', expression=0.005)
        k_d = gillespy2.Parameter(name='k_d', expression=0.08)
        self.add_parameter([k_c, k_d])

        # Define variables for the molecular species representing M & D.
        m = gillespy2.Species(name='monomer', initial_value=30)
        d = gillespy2.Species(name='dimer',   initial_value=0)
        self.add_species([m, d])

        # The list of reactants and products for a Reaction object are
        # each a Python dictionary in which the dictionary keys are
        # Species objects and the values are stoichiometries of the
        # species in the reaction.
        r_c = gillespy2.Reaction(name="r_creation", rate=k_c,
                                 reactants={m:2}, products={d:1})
        r_d = gillespy2.Reaction(name="r_dissociation", rate=k_d,
                                 reactants={d:1}, products={m:2})
        self.add_reaction([r_c, r_d])

        # Set the timespan for the simulation.
        self.timespan(numpy.linspace(0, 100, 101))

model = Dimerization()
p = pickle.dumps(model)
hash = hashlib.md5(p)

modelMD5 = hash.digest()
print(modelMD5)
#pickle.dump(model,open( f"./{modelMD5}.pkl", "wb" ))
#subprocess.run(["../remote-solver", f"{modelMD5}"])
