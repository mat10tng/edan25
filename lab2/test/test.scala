import akka.actor.Actor
import akka.actor.ActorSystem
import akka.actor.Props

class CountingActor(ind: int) extends Actor {
  var count = 0;
  val index =ind;


  def act() {
    react{
      case Count(plusone : int ) => {

      }



    }
  }
}
